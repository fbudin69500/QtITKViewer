CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )
CMAKE_POLICY(VERSION 2.6)
PROJECT(TestQt4)

find_package(Qt4 REQUIRED )
include(${QT_USE_FILE})

find_package( VTK REQUIRED )
include(${VTK_USE_FILE})

find_package( ITK REQUIRED )
include(${ITK_USE_FILE})

# Use the include path and library for Qt that is used by VTK.
INCLUDE_DIRECTORIES(
  ${QT_INCLUDE_DIR}
  ${TestQt4_BINARY_DIR}
  ${TestQt4_SOURCE_DIR}
)

QT4_WRAP_CPP(MOC_FILES TestQt4.h)
QT4_WRAP_UI(UI_FILES TestQt4.ui)

set(TestQt4_SOURCE main.cxx TestQt4.cxx ${MOC_FILES} ${UI_FILES})
add_executable( TestQt4 ${TestQt4_SOURCE})
target_link_libraries(TestQt4 ${QT_LIBRARIES} ${ITK_LIBRARIES} ${VTK_LIBRARIES} QVTK )
