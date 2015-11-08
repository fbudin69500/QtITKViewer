#ifndef TestQt4_h
#define TestQt4_h

#include <QFileDialog>
#include <QMimeData>
#include <QUrl>
#include <QDragEnterEvent>
#include <QMessageBox>
#include <QSignalMapper>

#include <vtkSmartPointer.h>
#include <vtkImageViewer2.h>
#include <vtkResliceImageViewer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkImageActor.h>
#include <vtkCornerAnnotation.h>
#include <vtkTextProperty.h>
//#include <vtkImagePlaneWidget.h>
#include <vtkLookupTable.h>
#include <vtkImageMapToColors.h>
#include <vtkImageMapper3D.h>

#include <vtkOutlineFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>

#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageToVTKImageFilter.h>
#include <itkMinimumMaximumImageCalculator.h>

#include "ui_TestQt4.h"

class TestQt4 :public QMainWindow, public Ui::MainWindow
{
  Q_OBJECT
  typedef itk::Image<float,3> ImageType ;
  typedef itk::ImageFileReader<ImageType> ReaderType ;
  public:
    TestQt4(QWidget * parent = 0, Qt::WFlags f = 0 );
  private slots:
   void slotExit();
   void slotLoad();
   void slotLoadMask() ;
   void slotSliceChanged(int value) ;
   void slotResetWindowLevel() ;
   void slotWindowLevelChanged() ;
   void slotSliceOrientation(int slice) ;
   void slotInterpolate() ;
  signals:
    void resetWindowLevel() ;
    void signalSliceOrientation(int slice) ;
  private:
    void FileChanged() ;
    void MaskFileChanged() ;
    std::string m_FileName ;
    std::string m_MaskFileName ;
    double scrollScalingConst ;
    ImageType::Pointer m_ITKImage ;
    double m_ImageMinIntensity ;
    double m_ImageMaxIntensity ;
    std::vector< std::string > m_SupportedExtensions ;
    QString m_listSupportedExtensions ;
    vtkSmartPointer<vtkImageViewer2> image_view;
  //  vtkSmartPointer<vtkImagePlaneWidget> planeWidget ;
    //vtkSmartPointer<vtkResliceImageViewer> image_view;
    void dropEvent(QDropEvent* Qevent) ;
    void dragEnterEvent(QDragEnterEvent *Qevent); // enable drag&drop
    vtkSmartPointer<vtkLookupTable> lookupTable ;
};

#endif
