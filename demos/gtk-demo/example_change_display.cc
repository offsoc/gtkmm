/* Change Display
 *
 * Demonstrates migrating a window between different displays and
 * screens. A display is a mouse and keyboard with some number of
 * associated monitors. A screen is a set of monitors grouped
 * into a single physical work area. The neat thing about having
 * multiple displays is that they can be on a completely separate
 * computers, as long as there is a network connection to the
 * computer where the application is running.
 *
 * Only some of the windowing systems where GTK+ runs have the
 * concept of multiple displays and screens. (The X Window System
 * is the main example.) Other windowing systems can only
 * handle one keyboard and mouse, and combine all monitors into
 * a single screen.
 *
 * This is a moderately complex example, and demonstrates:
 *
 *  - Tracking the currently open displays and screens
 *
 *  - Changing the screen for a window
 *
 *  - Letting the user choose a window by clicking on it
 *
 *  - Using Gtk::ListStore and Gtk::TreeView
 *
 *  - Using Gtk::Dialog
 */

#include <gtkmm.h>


class Popup : public Gtk::Window
{
public:
  Popup(const Glib::RefPtr<Gdk::Screen> screen, const Glib::ustring& prompt);
  virtual ~Popup();

protected:
  Gtk::Frame m_Frame;
  Gtk::Label m_Label;
};

class Example_ChangeDisplay : public Gtk::Dialog
{
public:
  Example_ChangeDisplay();
  virtual ~Example_ChangeDisplay();

protected:
  virtual void setup_frame(Gtk::Frame& frame, Gtk::TreeView& treeview, Gtk::Box& buttonbox);

  virtual void initialize_displays();
  virtual void fill_screens();
  virtual void query_change_display();
  virtual Gtk::Widget* find_toplevel_at_pointer(const Glib::RefPtr<Gdk::Display>& display);
  virtual Gtk::Window* query_for_toplevel(const Glib::RefPtr<Gdk::Screen>& screen, const Glib::ustring& prompt);

  //signal handlers:
  virtual void on_button_display_open();
  virtual void on_button_display_close();

  virtual void on_treeview_display_selection_changed();
  virtual void on_treeview_screen_selection_changed();

  virtual void on_display_closed(bool is_error, Glib::RefPtr<Gdk::Display> display);

  virtual bool on_popup_button_release_event(GdkEventButton* event);

  virtual void on_response(int response_id);


  class ModelColumns_Display : public Gtk::TreeModelColumnRecord
  {
  public:
    Gtk::TreeModelColumn<Glib::ustring> m_name;
    Gtk::TreeModelColumn< Glib::RefPtr<Gdk::Display> > m_display; //hidden

    ModelColumns_Display() { add(m_name); add(m_display); }
  };
  ModelColumns_Display m_columns_display;

  class ModelColumns_Screen : public Gtk::TreeModelColumnRecord
  {
  public:
    Gtk::TreeModelColumn<int> m_number;
    Gtk::TreeModelColumn< Glib::RefPtr<Gdk::Screen> > m_screen; //hidden

    ModelColumns_Screen() { add(m_number); add(m_screen); }
  };
  ModelColumns_Screen m_columns_screen;

  Gtk::VBox m_VBox;
  Gtk::Frame m_Frame_Display, m_Frame_Screen;
  Gtk::TreeView m_TreeView_Display, m_TreeView_Screen;
  Glib::RefPtr<Gtk::ListStore> m_refListStore_Display,  m_refListStore_Screen;
  Gtk::VBox m_ButtonBox_Display, m_ButtonBox_Screen;

  Gtk::Button m_Button_Display_Open, m_Button_Display_Close;

  Glib::RefPtr<Gtk::SizeGroup> m_refSizeGroup_Display, m_refSizeGroup_Screen;


  Glib::RefPtr<Gdk::Display> m_refCurrentDisplay;
  Glib::RefPtr<Gdk::Screen> m_refCurrentScreen;

  Popup* m_pPopup;

  bool m_popup_clicked;
};




Example_ChangeDisplay::Example_ChangeDisplay()
: Gtk::Dialog("Change Screen or display"),
  m_VBox(false, 5),
  m_Frame_Display("Display"),
  m_Frame_Screen("Screen"),
  m_ButtonBox_Display(false, 5), m_ButtonBox_Screen(false, 5),
  m_Button_Display_Open("_Open...", true), m_Button_Display_Close("_Close...", true),
  m_pPopup(0),
  m_popup_clicked(false)
{
  add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);
  add_button("Change", Gtk::RESPONSE_OK);

  set_default_size(300, 400);

  m_VBox.set_border_width(8);
  get_vbox()->pack_start(m_VBox);


  //Display:
  {
    setup_frame(m_Frame_Display, m_TreeView_Display, m_ButtonBox_Display);
    m_ButtonBox_Display.pack_start(m_Button_Display_Open, Gtk::PACK_SHRINK);
    m_Button_Display_Open.signal_clicked().connect( sigc::mem_fun(*this, &Example_ChangeDisplay::on_button_display_open) );
    m_ButtonBox_Display.pack_start(m_Button_Display_Close, Gtk::PACK_SHRINK);
    m_Button_Display_Close.signal_clicked().connect( sigc::mem_fun(*this, &Example_ChangeDisplay::on_button_display_close) );

    //Setup TreeView:
    m_refListStore_Display = Gtk::ListStore::create(m_columns_display);
    m_TreeView_Display.set_model(m_refListStore_Display);
    m_TreeView_Display.append_column("Name", m_columns_display.m_name);

    //Connect signal:
    Glib::RefPtr<Gtk::TreeView::Selection> refSelection = m_TreeView_Display.get_selection();
    refSelection->signal_changed().connect( sigc::mem_fun(*this, &Example_ChangeDisplay::on_treeview_display_selection_changed) );

    m_VBox.pack_start(m_Frame_Display);

    m_refSizeGroup_Display = Gtk::SizeGroup::create(Gtk::SIZE_GROUP_HORIZONTAL);
    m_refSizeGroup_Display->add_widget(m_ButtonBox_Display);
  }

  //Screen:
  {
    setup_frame(m_Frame_Screen, m_TreeView_Screen, m_ButtonBox_Screen);

    //Setup TreeView:
    m_refListStore_Screen = Gtk::ListStore::create(m_columns_screen);
    m_TreeView_Screen.set_model(m_refListStore_Screen);
    m_TreeView_Screen.append_column("Number", m_columns_screen.m_number);

    //Connect signal:
    Glib::RefPtr<Gtk::TreeView::Selection> refSelection = m_TreeView_Screen.get_selection();
    refSelection->signal_changed().connect( sigc::mem_fun(*this, &Example_ChangeDisplay::on_treeview_screen_selection_changed) );

    m_VBox.pack_start(m_Frame_Screen);

    m_refSizeGroup_Screen = Gtk::SizeGroup::create(Gtk::SIZE_GROUP_HORIZONTAL);
    m_refSizeGroup_Screen->add_widget(m_ButtonBox_Screen);
  }

  initialize_displays();

  show_all();
}

Example_ChangeDisplay::~Example_ChangeDisplay()
{
  if(m_pPopup)
  {
    delete m_pPopup;
    m_pPopup = 0;
  }
}

void Example_ChangeDisplay::setup_frame(Gtk::Frame& frame, Gtk::TreeView& treeview, Gtk::Box& buttonbox)
{
  Gtk::HBox* pHBox = Gtk::manage( new Gtk::HBox(false, 8) );
  pHBox->set_border_width(8);
  frame.add(*pHBox);

  Gtk::ScrolledWindow* pScrolledWindow = Gtk::manage( new Gtk::ScrolledWindow() );
  pScrolledWindow->set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
  pScrolledWindow->set_shadow_type(Gtk::SHADOW_IN);
  pHBox->pack_start(*pScrolledWindow);

  treeview.set_headers_visible(false);
  pScrolledWindow->add(treeview);

  Glib::RefPtr<Gtk::TreeView::Selection> refSelection = treeview.get_selection();
  refSelection->set_mode(Gtk::SELECTION_BROWSE);

  pHBox->pack_start(buttonbox, Gtk::PACK_SHRINK);
}

void Example_ChangeDisplay::initialize_displays()
{

#ifndef G_OS_WIN32
  Glib::RefPtr<Gdk::DisplayManager> refDisplayManager = Gdk::DisplayManager::get();

  typedef std::list< Glib::RefPtr<Gdk::Display> > type_listDisplays;
  type_listDisplays listDisplays = refDisplayManager->list_displays();

  for(type_listDisplays::iterator iter = listDisplays.begin(); iter != listDisplays.end(); ++iter)
  {
    Glib::RefPtr<Gdk::Display> refDisplay = *iter;

    Gtk::TreeRow row = *(m_refListStore_Display->append());
    row[m_columns_display.m_name] = refDisplay->get_name();;
    row[m_columns_display.m_display] = refDisplay;

    refDisplay->signal_closed().connect(
      sigc::bind<-1>( sigc::mem_fun(*this, &Example_ChangeDisplay::on_display_closed), refDisplay) );
  }

#endif
}

void Example_ChangeDisplay::on_display_closed(bool /* is_error */, Glib::RefPtr<Gdk::Display> display)
{
  Gtk::TreeModel::Children children = m_refListStore_Display->children();
  for(Gtk::TreeModel::iterator iter = children.begin(); iter != children.end(); ++iter)
  {
    Glib::RefPtr<Gdk::Display> refDisplay = (*iter)[m_columns_display.m_display];
    if(refDisplay == display)
    {
      m_refListStore_Display->erase(iter);
    }
  }
}

void Example_ChangeDisplay::on_button_display_open()
{
  Gtk::Dialog dialog("Open Display", true);
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);

  dialog.set_default_response(Gtk::RESPONSE_OK);

  Gtk::Entry entry;
  entry.set_activates_default();

  Gtk::Label label("Please enter the name of\nthe new display\n");

  dialog.get_vbox()->add(label);
  dialog.get_vbox()->add(entry);

  entry.grab_focus();

  dialog.show_all_children();

  Glib::RefPtr<Gdk::Display> refResult;
  while (!refResult)
  {
    gint response_id = dialog.run();
    if (response_id != Gtk::RESPONSE_OK)
      break;

    Glib::ustring new_screen_name = entry.get_text();

    if( !new_screen_name.empty() )
    {
      refResult = Gdk::Display::open(new_screen_name);
      if (!refResult)
      {
        label.set_text("Can't open display.\nplease try another one\n");
      }
    }
  }
}

void Example_ChangeDisplay::on_button_display_close()
{
  if(m_refCurrentDisplay)
    m_refCurrentDisplay->close();
}

void Example_ChangeDisplay::on_treeview_display_selection_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> refSelection = m_TreeView_Display.get_selection();
  Gtk::TreeModel::iterator iter = refSelection->get_selected();
  if(iter)
    m_refCurrentDisplay = (*iter)[m_columns_display.m_display];
  else
    m_refCurrentDisplay.clear();

  fill_screens();
}

void Example_ChangeDisplay::on_treeview_screen_selection_changed()
{
  Glib::RefPtr<Gtk::TreeSelection> refSelection = m_TreeView_Screen.get_selection();
  Gtk::TreeModel::iterator iter = refSelection->get_selected();
  if(iter)
    m_refCurrentScreen = (*iter)[m_columns_screen.m_screen];
  else
    m_refCurrentScreen.clear();
}

/* Fills in the screen list based on the current display
 */
void Example_ChangeDisplay::fill_screens()
{
  m_refListStore_Screen->clear();

  if(m_refCurrentScreen)
  {
    int n_screens = m_refCurrentDisplay->get_n_screens();

    for (int i = 0; i < n_screens; i++)
    {
      Glib::RefPtr<Gdk::Screen> refScreen = m_refCurrentDisplay->get_screen(i);

      Gtk::TreeModel::Row row = *(m_refListStore_Screen->append());
      row[m_columns_screen.m_number] = i;
      row[m_columns_screen.m_screen] = refScreen;

      if (i == 0)
      {
        Glib::RefPtr<Gtk::TreeSelection> refSelection = m_TreeView_Screen.get_selection();
        refSelection->select(row);
      }
    }
  }
}

/* Prompts the user for a toplevel window to move, and then moves
 * that window to the currently selected display
 */
void Example_ChangeDisplay::query_change_display()
{
  Glib::RefPtr<Gdk::Screen> refScreen = get_screen();
  Gtk::Window* pTopLevel = query_for_toplevel(refScreen,
   "Please select the toplevel\n"
   "to move to the new screen");

  if (pTopLevel)
    pTopLevel->set_screen(m_refCurrentScreen);
  else
    refScreen->get_display()->beep();
}


void Example_ChangeDisplay::on_response(int response_id)
{
  if (response_id == Gtk::RESPONSE_OK)
    query_change_display();
  else
    hide();
}



/* Asks the user to click on a window, then waits for them click
 * the mouse. When the mouse is released, returns the toplevel
 * window under the pointer, or NULL, if there is none.
 */
Gtk::Window* Example_ChangeDisplay::query_for_toplevel(const Glib::RefPtr<Gdk::Screen>& screen, const Glib::ustring& prompt)
{
  Glib::RefPtr<Gdk::Display> refDisplay = screen->get_display();

  if(m_pPopup)
  {
    delete m_pPopup;
    m_pPopup = 0;
  }

  m_pPopup = new Popup(screen, prompt);

  m_pPopup->show();

  Gdk::Cursor cursor(refDisplay, Gdk::CROSSHAIR);

  Gtk::Window* toplevel = 0;

  if( m_pPopup->get_window()->pointer_grab(false, Gdk::BUTTON_RELEASE_MASK, cursor, GDK_CURRENT_TIME)
     == Gdk::GRAB_SUCCESS )
  {
    m_popup_clicked = false;
    m_pPopup->signal_button_release_event().connect( sigc::mem_fun(*this, &Example_ChangeDisplay::on_popup_button_release_event) );


    // Process events until clicked is set by button_release_event_cb.
    // We pass in may_block=true since we want to wait if there
    // are no events currently.
    while (!m_popup_clicked)
      Gtk::Main::iteration(true);

    toplevel = dynamic_cast<Gtk::Window*>(find_toplevel_at_pointer(screen->get_display()));
    if (toplevel == m_pPopup)
       toplevel = 0;
  }

  Gdk::flush(); /* Really release the grab */

  return toplevel;
}

// Finds the toplevel window under the mouse pointer, if any.
Gtk::Widget* Example_ChangeDisplay::find_toplevel_at_pointer(const Glib::RefPtr<Gdk::Display>& display)
{
  Glib::RefPtr<Gdk::Window> refPointerWindow = display->get_window_at_pointer();

  if (refPointerWindow)
  {
    // The user data field of a GdkWindow is used to store a pointer
    // to the widget that created it.
    GtkWidget* cWidget = 0;
    gpointer* user_data = 0;
    refPointerWindow->get_user_data(user_data);
    cWidget = (GtkWidget*)user_data;

    Gtk::Widget* pWidget = Glib::wrap(cWidget);
    if(pWidget)
      return pWidget->get_toplevel();
  }

  return 0;
}


bool Example_ChangeDisplay::on_popup_button_release_event(GdkEventButton* /* event */)
{
  m_popup_clicked = true;
  return true;
}

Popup::Popup(const Glib::RefPtr<Gdk::Screen> screen, const Glib::ustring& prompt)
: Gtk::Window(Gtk::WINDOW_POPUP),
  m_Label(prompt)
{
  set_screen(screen);
  set_modal(true);
  set_position(Gtk::WIN_POS_CENTER);

  m_Frame.set_shadow_type(Gtk::SHADOW_OUT);
  add(m_Frame);

  m_Label.set_padding(10, 10);
  m_Frame.add(m_Label);

  show_all_children();
}

Popup::~Popup()
{
}


// called by DemoWindow
Gtk::Window* do_change_display()
{
  return new Example_ChangeDisplay();
}
