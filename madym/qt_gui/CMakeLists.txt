# Add folder where .h files are created (from the .ui files) to the search path
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} )

find_package(Qt5Core)
find_package(Qt5Widgets)

# Convert these .ui files to .h
QT5_WRAP_UI( madym_gui_ui_h_files
	madym_gui.ui
	madym_model_configure.ui
)	

# Convert these .h files to .cxx
QT5_WRAP_CPP( madym_gui_moc_files 
	madym_gui_ui.h
	madym_gui_processor.h
	madym_gui_model_configure.h
)

# Convert these resource files
QT5_ADD_RESOURCES( madym_gui_res_files
	madym_gui.qrc	
)

SET(madym_gui_sources
	madym_gui.cxx
	madym_gui_ui.h madym_gui_ui.cxx 
	madym_gui_processor.h madym_gui_processor.cxx
	madym_gui_model_configure.h madym_gui_model_configure.cxx
)

ADD_EXECUTABLE( madym_gui
	${madym_gui_sources}
	${madym_gui_ui_h_files}
	${madym_gui_moc_files} 
	${madym_gui_res_files}
)

TARGET_LINK_LIBRARIES( madym_gui mdm)

qt5_use_modules(madym_gui Widgets)
qt5_use_modules(madym_gui Core)
qt5_use_modules(madym_gui Gui)

