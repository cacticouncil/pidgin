
typedef GtkWidget *(*GDoDemoFunc) (GtkWidget *do_widget);

typedef struct _DemoData DemoData;

struct _DemoData
{
  const char *name;
  const char *title;
  const char **keywords;
  const char *filename;
  GDoDemoFunc func;
  DemoData *children;
};
GtkWidget *do_application_demo (GtkWidget *do_widget);
GtkWidget *do_assistant (GtkWidget *do_widget);
GtkWidget *do_builder (GtkWidget *do_widget);
GtkWidget *do_clipboard (GtkWidget *do_widget);
GtkWidget *do_combobox (GtkWidget *do_widget);
GtkWidget *do_constraints (GtkWidget *do_widget);
GtkWidget *do_constraints_interactive (GtkWidget *do_widget);
GtkWidget *do_constraints_vfl (GtkWidget *do_widget);
GtkWidget *do_constraints_builder (GtkWidget *do_widget);
GtkWidget *do_css_accordion (GtkWidget *do_widget);
GtkWidget *do_css_basics (GtkWidget *do_widget);
GtkWidget *do_css_blendmodes (GtkWidget *do_widget);
GtkWidget *do_css_multiplebgs (GtkWidget *do_widget);
GtkWidget *do_css_pixbufs (GtkWidget *do_widget);
GtkWidget *do_css_shadows (GtkWidget *do_widget);
GtkWidget *do_cursors (GtkWidget *do_widget);
GtkWidget *do_dialog (GtkWidget *do_widget);
GtkWidget *do_drawingarea (GtkWidget *do_widget);
GtkWidget *do_dnd (GtkWidget *do_widget);
GtkWidget *do_editable_cells (GtkWidget *do_widget);
GtkWidget *do_entry_completion (GtkWidget *do_widget);
GtkWidget *do_entry_undo (GtkWidget *do_widget);
GtkWidget *do_errorstates (GtkWidget *do_widget);
GtkWidget *do_expander (GtkWidget *do_widget);
GtkWidget *do_filtermodel (GtkWidget *do_widget);
GtkWidget *do_fishbowl (GtkWidget *do_widget);
GtkWidget *do_fixed (GtkWidget *do_widget);
GtkWidget *do_fontrendering (GtkWidget *do_widget);
GtkWidget *do_frames (GtkWidget *do_widget);
GtkWidget *do_gears (GtkWidget *do_widget);
GtkWidget *do_gestures (GtkWidget *do_widget);
GtkWidget *do_glarea (GtkWidget *do_widget);
GtkWidget *do_gltransition (GtkWidget *do_widget);
GtkWidget *do_headerbar (GtkWidget *do_widget);
GtkWidget *do_hypertext (GtkWidget *do_widget);
GtkWidget *do_iconscroll (GtkWidget *do_widget);
GtkWidget *do_iconview (GtkWidget *do_widget);
GtkWidget *do_iconview_edit (GtkWidget *do_widget);
GtkWidget *do_images (GtkWidget *do_widget);
GtkWidget *do_infobar (GtkWidget *do_widget);
GtkWidget *do_layoutmanager (GtkWidget *do_widget);
GtkWidget *do_layoutmanager2 (GtkWidget *do_widget);
GtkWidget *do_links (GtkWidget *do_widget);
GtkWidget *do_listbox (GtkWidget *do_widget);
GtkWidget *do_listbox_controls (GtkWidget *do_widget);
GtkWidget *do_menu (GtkWidget *do_widget);
GtkWidget *do_flowbox (GtkWidget *do_widget);
GtkWidget *do_list_store (GtkWidget *do_widget);
GtkWidget *do_listview_applauncher (GtkWidget *do_widget);
GtkWidget *do_listview_clocks (GtkWidget *do_widget);
GtkWidget *do_listview_colors (GtkWidget *do_widget);
GtkWidget *do_listview_filebrowser (GtkWidget *do_widget);
GtkWidget *do_listview_minesweeper (GtkWidget *do_widget);
GtkWidget *do_dropdown (GtkWidget *do_widget);
GtkWidget *do_listview_settings (GtkWidget *do_widget);
GtkWidget *do_listview_ucd (GtkWidget *do_widget);
GtkWidget *do_listview_weather (GtkWidget *do_widget);
GtkWidget *do_listview_words (GtkWidget *do_widget);
GtkWidget *do_markup (GtkWidget *do_widget);
GtkWidget *do_overlay (GtkWidget *do_widget);
GtkWidget *do_overlay_decorative (GtkWidget *do_widget);
GtkWidget *do_paint (GtkWidget *do_widget);
GtkWidget *do_paintable (GtkWidget *do_widget);
GtkWidget *do_paintable_animated (GtkWidget *do_widget);
GtkWidget *do_paintable_emblem (GtkWidget *do_widget);
GtkWidget *do_paintable_mediastream (GtkWidget *do_widget);
GtkWidget *do_paintable_symbolic (GtkWidget *do_widget);
GtkWidget *do_panes (GtkWidget *do_widget);
GtkWidget *do_password_entry (GtkWidget *do_widget);
GtkWidget *do_peg_solitaire (GtkWidget *do_widget);
GtkWidget *do_pickers (GtkWidget *do_widget);
GtkWidget *do_printing (GtkWidget *do_widget);
GtkWidget *do_revealer (GtkWidget *do_widget);
GtkWidget *do_rotated_text (GtkWidget *do_widget);
GtkWidget *do_scale (GtkWidget *do_widget);
GtkWidget *do_search_entry (GtkWidget *do_widget);
GtkWidget *do_search_entry2 (GtkWidget *do_widget);
GtkWidget *do_shadertoy (GtkWidget *do_widget);
GtkWidget *do_shortcuts (GtkWidget *do_widget);
GtkWidget *do_shortcut_triggers (GtkWidget *do_widget);
GtkWidget *do_sidebar (GtkWidget *do_widget);
GtkWidget *do_sizegroup (GtkWidget *do_widget);
GtkWidget *do_sliding_puzzle (GtkWidget *do_widget);
GtkWidget *do_spinbutton (GtkWidget *do_widget);
GtkWidget *do_spinner (GtkWidget *do_widget);
GtkWidget *do_stack (GtkWidget *do_widget);
GtkWidget *do_tabs (GtkWidget *do_widget);
GtkWidget *do_tagged_entry (GtkWidget *do_widget);
GtkWidget *do_textmask (GtkWidget *do_widget);
GtkWidget *do_textundo (GtkWidget *do_widget);
GtkWidget *do_textview (GtkWidget *do_widget);
GtkWidget *do_textscroll (GtkWidget *do_widget);
GtkWidget *do_themes (GtkWidget *do_widget);
GtkWidget *do_theming_style_classes (GtkWidget *do_widget);
GtkWidget *do_transparent (GtkWidget *do_widget);
GtkWidget *do_tree_store (GtkWidget *do_widget);
GtkWidget *do_video_player (GtkWidget *do_widget);
GtkWidget *do_font_features (GtkWidget *do_widget);
GtkWidget *do_pagesetup (GtkWidget *do_widget);

DemoData child0[] = {
  { "constraints", "Simple Constraints", (const char*[]) {"gtklayoutmanager", "gtkconstraintlayout", NULL }, "constraints.c", do_constraints, NULL },
  { "constraints_interactive", "Interactive Constraints", (const char*[]) {"gtkconstraintlayout", NULL }, "constraints_interactive.c", do_constraints_interactive, NULL },
  { "constraints_vfl", "VFL", (const char*[]) {"gtkconstraintlayout", NULL }, "constraints_vfl.c", do_constraints_vfl, NULL },
  { "constraints_builder", "Builder", (const char*[]) {"gtkconstraintlayouts", NULL }, "constraints_builder.c", do_constraints_builder, NULL },
  { NULL }
};

DemoData child4[] = {
  { "css_accordion", "CSS Accordion", (const char*[]) {NULL }, "css_accordion.c", do_css_accordion, NULL },
  { "css_basics", "CSS Basics", (const char*[]) {NULL }, "css_basics.c", do_css_basics, NULL },
  { "css_blendmodes", "CSS Blend Modes", (const char*[]) {NULL }, "css_blendmodes.c", do_css_blendmodes, NULL },
  { "css_multiplebgs", "Multiple Backgrounds", (const char*[]) {NULL }, "css_multiplebgs.c", do_css_multiplebgs, NULL },
  { "css_pixbufs", "Animated Backgrounds", (const char*[]) {NULL }, "css_pixbufs.c", do_css_pixbufs, NULL },
  { "css_shadows", "Shadows", (const char*[]) {NULL }, "css_shadows.c", do_css_shadows, NULL },
  { "theming_style_classes", "Style Classes", (const char*[]) {NULL }, "theming_style_classes.c", do_theming_style_classes, NULL },
  { NULL }
};

DemoData child10[] = {
  { "editable_cells", "Editable Cells", (const char*[]) {"gtktreeview", "gtkcellrenderertext", "gtkcellrendererprogress", "gtkcellrenderer::editing-started", "gtkliststore", "gtkcellrenderercombo", NULL }, "editable_cells.c", do_editable_cells, NULL },
  { "filtermodel", "Filter Model", (const char*[]) {"gtktreeview", "gtktreemodelfilter", NULL }, "filtermodel.c", do_filtermodel, NULL },
  { "list_store", "List Store", (const char*[]) {"gtktreeview", "gtkliststore", NULL }, "list_store.c", do_list_store, NULL },
  { "tree_store", "Tree Store", (const char*[]) {"gtktreeview", "gtktreestore", "gtkliststore", NULL }, "tree_store.c", do_tree_store, NULL },
  { NULL }
};

DemoData child11[] = {
  { "entry_completion", "Completion", (const char*[]) {"gtkentry", "gtkentrycompletion", NULL }, "entry_completion.c", do_entry_completion, NULL },
  { "entry_undo", "Undo and Redo", (const char*[]) {"gtkentry", NULL }, "entry_undo.c", do_entry_undo, NULL },
  { "password_entry", "Password Entry", (const char*[]) {"gtkpasswordentry", NULL }, "password_entry.c", do_password_entry, NULL },
  { "search_entry", "Search Entry", (const char*[]) {"gtkentry", NULL }, "search_entry.c", do_search_entry, NULL },
  { "search_entry2", "Type to Search", (const char*[]) {"gtksearchbar", "gtksearchentry", NULL }, "search_entry2.c", do_search_entry2, NULL },
  { "tagged_entry", "Tagged Entry", (const char*[]) {"gtktext", NULL }, "tagged_entry.c", do_tagged_entry, NULL },
  { NULL }
};

DemoData child14[] = {
  { "fishbowl", "Fishbowl", (const char*[]) {NULL }, "fishbowl.c", do_fishbowl, NULL },
  { "frames", "Frames", (const char*[]) {NULL }, "frames.c", do_frames, NULL },
  { "iconscroll", "Scrolling", (const char*[]) {"gtkscrolledwindow", NULL }, "iconscroll.c", do_iconscroll, NULL },
  { "themes", "Themes", (const char*[]) {NULL }, "themes.c", do_themes, NULL },
  { NULL }
};

DemoData child15[] = {
  { "fontrendering", "Font Rendering", (const char*[]) {NULL }, "fontrendering.c", do_fontrendering, NULL },
  { "rotated_text", "Rotated Text", (const char*[]) {"pangocairo", "gtklabel", NULL }, "rotated_text.c", do_rotated_text, NULL },
  { "textmask", "Text Mask", (const char*[]) {"pangocairo", NULL }, "textmask.c", do_textmask, NULL },
  { "font_features", "Font Explorer", (const char*[]) {"pango", NULL }, "font_features.c", do_font_features, NULL },
  { NULL }
};

DemoData child17[] = {
  { "gears", "Gears", (const char*[]) {"gtkglarea", NULL }, "gears.c", do_gears, NULL },
  { "glarea", "OpenGL Area", (const char*[]) {"gtkglarea", NULL }, "glarea.c", do_glarea, NULL },
  { "gltransition", "Transitions and Effects", (const char*[]) {"effect", "opengl", "gl", "shader", NULL }, "gltransition.c", do_gltransition, NULL },
  { "shadertoy", "Shadertoy", (const char*[]) {"gtkglarea", NULL }, "shadertoy.c", do_shadertoy, NULL },
  { NULL }
};

DemoData child20[] = {
  { "hypertext", "Hypertext", (const char*[]) {"gtktextbuffer", "gtktextview", NULL }, "hypertext.c", do_hypertext, NULL },
  { "markup", "Markup", (const char*[]) {"pango", "gtktextbuffer", "gtktextview", NULL }, "markup.c", do_markup, NULL },
  { "tabs", "Tabs", (const char*[]) {"gtktextview", NULL }, "tabs.c", do_tabs, NULL },
  { "textundo", "Undo and Redo", (const char*[]) {"gtktextbuffer", "gtktextview", NULL }, "textundo.c", do_textundo, NULL },
  { "textview", "Multiple Views", (const char*[]) {"gtktextbuffer", "gtktextview", "gtktextviews", NULL }, "textview.c", do_textview, NULL },
  { "textscroll", "Automatic Scrolling", (const char*[]) {"gtkscrolledwindow", "gtktextview", "gtktextmarks", NULL }, "textscroll.c", do_textscroll, NULL },
  { NULL }
};

DemoData child22[] = {
  { "iconview", "Icon View Basics", (const char*[]) {"gtkiconview", "gtktreemodel", NULL }, "iconview.c", do_iconview, NULL },
  { "iconview_edit", "Editing and Drag-and-Drop", (const char*[]) {"gtkcelllayout", "gtkiconview", NULL }, "iconview_edit.c", do_iconview_edit, NULL },
  { NULL }
};

DemoData child24[] = {
  { "layoutmanager", "Transition", (const char*[]) {"gtklayoutmanager", NULL }, "layoutmanager.c", do_layoutmanager, NULL },
  { "layoutmanager2", "Transformation", (const char*[]) {"gtklayoutmanager", "gsktransform", NULL }, "layoutmanager2.c", do_layoutmanager2, NULL },
  { NULL }
};

DemoData child26[] = {
  { "listbox", "Complex", (const char*[]) {"gtklistbox", NULL }, "listbox.c", do_listbox, NULL },
  { "listbox_controls", "Controls", (const char*[]) {"gtklistbox", NULL }, "listbox_controls.c", do_listbox_controls, NULL },
  { NULL }
};

DemoData child29[] = {
  { "listview_applauncher", "Application launcher", (const char*[]) {"gtklistview", "glistmodel", "gtklistitemfactory", NULL }, "listview_applauncher.c", do_listview_applauncher, NULL },
  { "listview_clocks", "Clocks", (const char*[]) {"glistmodel", "gtkgridview", "gtklistitemfactory", "gtkbuilder", NULL }, "listview_clocks.c", do_listview_clocks, NULL },
  { "listview_colors", "Colors", (const char*[]) {"gtkgridview", "gtkmultiselection", "gtksortlistmodel", "gtkdropdown", NULL }, "listview_colors.c", do_listview_colors, NULL },
  { "listview_filebrowser", "File browser", (const char*[]) {"glistmodel", "gtklistview", "gtkgridview", NULL }, "listview_filebrowser.c", do_listview_filebrowser, NULL },
  { "listview_minesweeper", "Minesweeper", (const char*[]) {"glistmodel", "gtkgridview", "game", NULL }, "listview_minesweeper.c", do_listview_minesweeper, NULL },
  { "dropdown", "Selections", (const char*[]) {"gtkcombobox", "gtkcomboboxtext", "gtkdropdown", "gtkentrycompletion", NULL }, "dropdown.c", do_dropdown, NULL },
  { "listview_settings", "Settings", (const char*[]) {"gtkcolumnview", "gtklistitemfactory", "gtklistview", "glistmodel", "gsettings", NULL }, "listview_settings.c", do_listview_settings, NULL },
  { "listview_ucd", "Characters", (const char*[]) {NULL }, "listview_ucd.c", do_listview_ucd, NULL },
  { "listview_weather", "Weather", (const char*[]) {"gtklistview", "gtkorientable", "gtknoselectionmodel", NULL }, "listview_weather.c", do_listview_weather, NULL },
  { "listview_words", "Words", (const char*[]) {"gtklistview", "gtkfilterlistmodel", NULL }, "listview_words.c", do_listview_words, NULL },
  { NULL }
};

DemoData child40[] = {
  { "overlay", "Interactive Overlay", (const char*[]) {"gtkoverlay", NULL }, "overlay.c", do_overlay, NULL },
  { "overlay_decorative", "Decorative Overlay", (const char*[]) {"gtkoverlay", NULL }, "overlay_decorative.c", do_overlay_decorative, NULL },
  { "transparent", "Transparency", (const char*[]) {"gtksnapshot", "gtkoverlay", NULL }, "transparent.c", do_transparent, NULL },
  { NULL }
};

DemoData child42[] = {
  { "paintable", "Simple Paintable", (const char*[]) {"gdkpaintable", "gtkimage", NULL }, "paintable.c", do_paintable, NULL },
  { "paintable_animated", "Animated Paintable", (const char*[]) {"gdkpaintable", NULL }, "paintable_animated.c", do_paintable_animated, NULL },
  { "paintable_emblem", "Emblems", (const char*[]) {"gdkpaintable", NULL }, "paintable_emblem.c", do_paintable_emblem, NULL },
  { "paintable_mediastream", "Media Stream", (const char*[]) {"gdkpaintable", "gtkmediastream", NULL }, "paintable_mediastream.c", do_paintable_mediastream, NULL },
  { "paintable_symbolic", "Symbolic Paintable", (const char*[]) {"gtksymbolicpaintable", "gdkpaintables", NULL }, "paintable_symbolic.c", do_paintable_symbolic, NULL },
  { NULL }
};

DemoData child48[] = {
  { "printing", "Printing", (const char*[]) {"gtkprintoperation", NULL }, "printing.c", do_printing, NULL },
  { "pagesetup", "Page Setup", (const char*[]) {"gtkpagesetupunixdialog", "gtkpagesetup", NULL }, "pagesetup.c", do_pagesetup, NULL },
  { NULL }
};

DemoData gtk_demos[] = {
  { "application_demo", "Application Class", (const char*[]) {"gtkapplication", "gresource", "gtkapplicationwindow", "gtkbuilder", "gmenu", NULL }, "application_demo.c", do_application_demo, NULL },
  { "assistant", "Assistant", (const char*[]) {"gtkassistant", NULL }, "assistant.c", do_assistant, NULL },
  { NULL, "Benchmark", (const char*[]) {NULL }, NULL, NULL, child14 },
  { "builder", "Builder", (const char*[]) {"gtkshortcutcontroller", "toolbar", "gtkpopovermenubar", "gtkbuilder", "gmenu", "gtkstatusbar", NULL }, "builder.c", do_builder, NULL },
  { "clipboard", "Clipboard", (const char*[]) {"gdkclipboard", NULL }, "clipboard.c", do_clipboard, NULL },
  { "combobox", "Combo Boxes", (const char*[]) {"gtkcombobox", "gtkcellrenderer", "gtkcomboboxentry", NULL }, "combobox.c", do_combobox, NULL },
  { NULL, "Constraints", (const char*[]) {NULL }, NULL, NULL, child0 },
  { "cursors", "Cursors", (const char*[]) {NULL }, "cursors.c", do_cursors, NULL },
  { "dialog", "Dialogs", (const char*[]) {"gtkmessagedialog", NULL }, "dialog.c", do_dialog, NULL },
  { "dnd", "Drag-and-Drop", (const char*[]) {"popover", "menu", "gesture", "dnd", NULL }, "dnd.c", do_dnd, NULL },
  { "drawingarea", "Drawing Area", (const char*[]) {"gtkdrawingarea", NULL }, "drawingarea.c", do_drawingarea, NULL },
  { NULL, "Entry", (const char*[]) {NULL }, NULL, NULL, child11 },
  { "errorstates", "Error States", (const char*[]) {"gtkentry", "gtklabel", "gtkbuilderscope", "gtkbuilder", NULL }, "errorstates.c", do_errorstates, NULL },
  { "expander", "Expander", (const char*[]) {"gtkexpander", NULL }, "expander.c", do_expander, NULL },
  { "fixed", "Fixed Layout", (const char*[]) {"gtklayoutmanager", "gtkfixed", NULL }, "fixed.c", do_fixed, NULL },
  { "flowbox", "Flow Box", (const char*[]) {"gtkflowbox", NULL }, "flowbox.c", do_flowbox, NULL },
  { "gestures", "Gestures", (const char*[]) {"gtkgesture", NULL }, "gestures.c", do_gestures, NULL },
  { "headerbar", "Header Bar", (const char*[]) {"gtkwindowcontrols", "gtkwindowhandle", "gtkheaderbar", NULL }, "headerbar.c", do_headerbar, NULL },
  { NULL, "Icon View", (const char*[]) {NULL }, NULL, NULL, child22 },
  { "images", "Images", (const char*[]) {"gtkpicture", "gtkwidgetpaintable", "gdkpaintable", "gtkimage", NULL }, "images.c", do_images, NULL },
  { "infobar", "Info Bars", (const char*[]) {"gtkinfobar", NULL }, "infobar.c", do_infobar, NULL },
  { NULL, "Layout Manager", (const char*[]) {NULL }, NULL, NULL, child24 },
  { "links", "Links", (const char*[]) {"gtklabel", NULL }, "links.c", do_links, NULL },
  { NULL, "List Box", (const char*[]) {NULL }, NULL, NULL, child26 },
  { NULL, "Lists", (const char*[]) {NULL }, NULL, NULL, child29 },
  { "menu", "Menu", (const char*[]) {"gtkpicture", "zoom", "action", NULL }, "menu.c", do_menu, NULL },
  { NULL, "OpenGL", (const char*[]) {NULL }, NULL, NULL, child17 },
  { NULL, "Overlay", (const char*[]) {NULL }, NULL, NULL, child40 },
  { "paint", "Paint", (const char*[]) {"gdkdrawingarea", "gtkgesture", NULL }, "paint.c", do_paint, NULL },
  { NULL, "Paintable", (const char*[]) {NULL }, NULL, NULL, child42 },
  { "panes", "Paned Widgets", (const char*[]) {"gtkpaned", NULL }, "panes.c", do_panes, NULL },
  { NULL, "Pango", (const char*[]) {NULL }, NULL, NULL, child15 },
  { "peg_solitaire", "Peg Solitaire", (const char*[]) {"gtkgridview", "game", NULL }, "peg_solitaire.c", do_peg_solitaire, NULL },
  { "pickers", "Pickers", (const char*[]) {"gtkapplicationchooser", "gtkfontchooser", "gtkcolorchooser", NULL }, "pickers.c", do_pickers, NULL },
  { NULL, "Printing", (const char*[]) {NULL }, NULL, NULL, child48 },
  { "revealer", "Revealer", (const char*[]) {"gtkrevealer", NULL }, "revealer.c", do_revealer, NULL },
  { "scale", "Scales", (const char*[]) {"gtkscale", NULL }, "scale.c", do_scale, NULL },
  { "shortcut_triggers", "Shortcuts", (const char*[]) {"gtkshortcut", "gtkshortcutcontroller", NULL }, "shortcut_triggers.c", do_shortcut_triggers, NULL },
  { "shortcuts", "Shortcuts Window", (const char*[]) {"gtkshortcutswindow", NULL }, "shortcuts.c", do_shortcuts, NULL },
  { "sizegroup", "Size Groups", (const char*[]) {"gtksizegroup", "gtktable", "gtk_fill", NULL }, "sizegroup.c", do_sizegroup, NULL },
  { "sliding_puzzle", "Sliding Puzzle", (const char*[]) {"game", "gdkpaintable", "gtkshortcutcontroller", "gdkgesture", NULL }, "sliding_puzzle.c", do_sliding_puzzle, NULL },
  { "spinbutton", "Spin Buttons", (const char*[]) {"gtkentry", "gtkspinbutton", NULL }, "spinbutton.c", do_spinbutton, NULL },
  { "spinner", "Spinner", (const char*[]) {"gtkspinner", NULL }, "spinner.c", do_spinner, NULL },
  { "stack", "Stack", (const char*[]) {"gtkstack", "gtkstackswitcher", NULL }, "stack.c", do_stack, NULL },
  { "sidebar", "Stack Sidebar", (const char*[]) {"gtkstack", "gtkstacksidebar", NULL }, "sidebar.c", do_sidebar, NULL },
  { NULL, "Text View", (const char*[]) {NULL }, NULL, NULL, child20 },
  { NULL, "Theming", (const char*[]) {NULL }, NULL, NULL, child4 },
  { NULL, "Tree View", (const char*[]) {NULL }, NULL, NULL, child10 },
  { "video_player", "Video Player", (const char*[]) {"gtkvideo", "gtkmediafile", "gdkpaintable", "gtkmediacontrols", "gtkmediastream", NULL }, "video_player.c", do_video_player, NULL },
  { NULL }
};
