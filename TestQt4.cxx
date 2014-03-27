#include "TestQt4.h"

TestQt4::TestQt4(QWidget * parent , Qt::WFlags f  ): QMainWindow(parent, f)
{
  setupUi(this);
  connect(this->exitButton, SIGNAL(clicked()), this, SLOT(slotExit()));
  connect(this->loadButton, SIGNAL(clicked()), this, SLOT(slotLoad()));
  connect(this->maskButton, SIGNAL(clicked()), this, SLOT(slotLoadMask()));
  connect(this->sliceScroll, SIGNAL(valueChanged(int)),this,SLOT(slotSliceChanged(int)));
  QSignalMapper *signalMapper = new QSignalMapper(this);
  connect(signalMapper, SIGNAL(mapped(int)), this, SIGNAL(signalSliceOrientation(int)));
  connect(this, SIGNAL(signalSliceOrientation(int)), this, SLOT(slotSliceOrientation(int)));
  signalMapper->setMapping(this->XradioButton, 0);
  connect(this->XradioButton, SIGNAL(clicked()), signalMapper, SLOT(map()));
  signalMapper->setMapping(this->YradioButton, 1);
  connect(this->YradioButton, SIGNAL(clicked()), signalMapper, SLOT(map()));
  signalMapper->setMapping(this->ZradioButton, 2);
  connect(this->ZradioButton, SIGNAL(clicked()), signalMapper, SLOT(map()));
  connect(this->windowScroll, SIGNAL(valueChanged(int)),this,SLOT(slotWindowLevelChanged()));
  connect(this->levelScroll, SIGNAL(valueChanged(int)),this,SLOT(slotWindowLevelChanged()));
  connect(this,SIGNAL(resetWindowLevel()), this,  SLOT(slotResetWindowLevel()) ) ;
  connect(this->resetWindowLevelButton, SIGNAL(clicked()), this, SLOT(slotResetWindowLevel()) ) ;
  connect(this->interpolateCheckBox, SIGNAL(stateChanged(int)), this, SLOT(slotInterpolate()) ) ;
  m_SupportedExtensions.push_back( "nrrd" ) ;
  m_SupportedExtensions.push_back( "nhdr" ) ;
  m_SupportedExtensions.push_back( "gipl" ) ;
  m_SupportedExtensions.push_back( "gipl.gz" ) ;
  m_SupportedExtensions.push_back( "nii" ) ;
  m_SupportedExtensions.push_back( "nii.gz" ) ;
  m_SupportedExtensions.push_back( "mha" ) ;
  m_SupportedExtensions.push_back( "mhd" ) ;
  m_SupportedExtensions.push_back( "hdr" ) ;
  m_listSupportedExtensions = tr( "Image Files (" ) ;
  for( size_t i = 0 ; i < m_SupportedExtensions.size() ; i++ )
  {
    m_listSupportedExtensions += tr( QString(" *.%1" ).arg(m_SupportedExtensions[ i ].c_str() ).toStdString().c_str() ) ;
  }
  m_listSupportedExtensions += tr( ")" ) ;
  this->setAcceptDrops(true);
  image_view = vtkSmartPointer<vtkImageViewer2>::New();

  //set VTK Viewer to QVTKWidget in Qt's UI
//  image_view->Render();
  //planeWidget = vtkSmartPointer<vtkImagePlaneWidget>::New();
  //image_view = vtkSmartPointer<vtkResliceImageViewer>::New();
  m_ITKImage = ImageType::New() ;
  scrollScalingConst = 255.0 ;
  sliceScroll->setMinimum(0);
  sliceScroll->setMaximum(0);
  levelScroll->setMinimum(0);
  levelScroll->setMaximum( scrollScalingConst );
  windowScroll->setMinimum(0);
  windowScroll->setMaximum( scrollScalingConst );
  windowLevelGroup->hide();
  m_ImageMaxIntensity = 0.0 ;
  m_ImageMinIntensity = 0.0 ;
  radioButtons->hide();
  sliceScroll->hide();
  interpolateCheckBox->hide();


  lookupTable = vtkSmartPointer<vtkLookupTable>::New() ;
   lookupTable->SetNumberOfTableValues(2);
   lookupTable->SetRange(0.0,1.0);
   lookupTable->SetTableValue( 0, 0.0, 0.0, 0.0, 0.0 ); //label 0 is transparent
   lookupTable->SetTableValue( 1, 0.0, 1.0, 0.0, 1.0 ); //label 1 is opaque and green
   lookupTable->Build();
   maskActor = vtkSmartPointer<vtkImageActor>::New();
}

void TestQt4::slotSliceOrientation(int slice)
{
    image_view->SetSliceOrientation(slice);
    sliceScroll->setMaximum( image_view->GetSliceMax() ) ;
    image_view->SetSlice(image_view->GetSliceMax()/2);
    sliceScroll->setValue(image_view->GetSliceMax()/2);
    visu->update() ;
}

void TestQt4::slotWindowLevelChanged()
{
  image_view->SetColorWindow( windowScroll->value()/scrollScalingConst*(m_ImageMaxIntensity - m_ImageMinIntensity) ) ;
  image_view->SetColorLevel( levelScroll->value()/scrollScalingConst*(m_ImageMaxIntensity - m_ImageMinIntensity ) + m_ImageMinIntensity ) ;
  visu->update() ;
}

void TestQt4::slotResetWindowLevel()
{
    windowScroll->setValue( scrollScalingConst ) ;
    levelScroll->setValue( scrollScalingConst/2 ) ;
    slotWindowLevelChanged() ;
}

void TestQt4::slotExit()
{
  qApp->exit();
}

void TestQt4::slotLoad()
{
  QString browseMesh = QFileDialog::getOpenFileName( this , tr("Open an image") , QString() , m_listSupportedExtensions );
  if( ! browseMesh.isEmpty() )
  {
    m_FileName = browseMesh.toStdString() ;
    FileChanged() ;
  }
}

void TestQt4::slotLoadMask()
{
    QString browseMesh = QFileDialog::getOpenFileName( this , tr("Open a mask") , QString() , m_listSupportedExtensions );
    if( ! browseMesh.isEmpty() )
    {
      m_MaskFileName = browseMesh.toStdString() ;
      MaskFileChanged() ;
    }
}

void TestQt4::MaskFileChanged()
{
  ReaderType::Pointer reader = ReaderType::New() ;
  reader->SetFileName( m_MaskFileName.c_str() ) ;
  try
  {
    reader->Update() ;
  }
  catch(...)
  {
     QMessageBox msgBox;
     msgBox.setText("Error while loading image");
     msgBox.exec();
     return ;
  }
  //connector to convert ITK image data to VTK image data
  typedef itk::ImageToVTKImageFilter<ImageType> ConnectorType;
  ConnectorType::Pointer connector= ConnectorType::New();
  connector->SetInput( reader->GetOutput() );//Set ITK reader Output to connector you can replace it with filter
  //Exceptional handling
  try
  {
    connector->Update();
  }
  catch (itk::ExceptionObject & e)
  {
    std::cerr << "exception in file reader " << std::endl;
    std::cerr << e << std::endl;
    return ;
  }
  //deep copy connector's output to an image else connector will go out of scope
  //and vanish it will cause error while changing slice
  vtkImageData * image = vtkImageData::New();
  image->DeepCopy(connector->GetOutput());

  vtkSmartPointer<vtkImageMapToColors> mapTransparency =
  vtkSmartPointer<vtkImageMapToColors>::New();
  mapTransparency->SetLookupTable(lookupTable);
  mapTransparency->PassAlphaToOutputOn();
  mapTransparency->SetInput(image);

  maskActor->GetMapper()->SetInputConnection( mapTransparency->GetOutputPort() ) ;
  image_view->GetRenderer()->AddActor(maskActor);
  image_view->GetRenderer()->ResetCamera();
  image_view->Render();
  visu->update() ;
}


void TestQt4::slotInterpolate()
{
  vtkImageActor* imageActor = image_view->GetImageActor() ;
  imageActor->SetInterpolate( interpolateCheckBox->isChecked() ) ;
}


void TestQt4::slotSliceChanged(int value)
{
  image_view->SetSlice(value);
  visu->update() ;
}

void TestQt4::FileChanged()
{
  ReaderType::Pointer reader = ReaderType::New() ;
  reader->SetFileName( m_FileName.c_str() ) ;
  try
  {
    reader->Update() ;
  }
  catch(...)
  {
     QMessageBox msgBox;
     msgBox.setText("Error while loading image");
     msgBox.exec();
     return ;
  }
  m_ITKImage = reader->GetOutput() ;
  typedef itk::MinimumMaximumImageCalculator< ImageType > MinMaxCalculatorType ;
  MinMaxCalculatorType::Pointer minMaxCalculator = MinMaxCalculatorType::New() ;
  minMaxCalculator->SetImage( m_ITKImage ) ;
  minMaxCalculator->Compute() ;
  m_ImageMaxIntensity = minMaxCalculator->GetMaximum() ;
  m_ImageMinIntensity = minMaxCalculator->GetMinimum() ;
  //connector to convert ITK image data to VTK image data
  typedef itk::ImageToVTKImageFilter<ImageType> ConnectorType;
  ConnectorType::Pointer connector= ConnectorType::New();
  connector->SetInput( m_ITKImage );//Set ITK reader Output to connector you can replace it with filter
  //Exceptional handling
  try
  {
    connector->Update();
  }
  catch (itk::ExceptionObject & e)
  {
    std::cerr << "exception in file reader " << std::endl;
    std::cerr << e << std::endl;
    return ;
  }
  //deep copy connector's output to an image else connector will go out of scope
  //and vanish it will cause error while changing slice
  vtkImageData * image = vtkImageData::New();
  image->DeepCopy(connector->GetOutput());
     
  //Set input image to VTK viewer
  image_view->SetInput(image);

  vtkSmartPointer<vtkImageActor> imageActor =
      vtkSmartPointer<vtkImageActor>::New();
    imageActor->SetInput(image);
  image_view->GetRenderer()->AddActor(imageActor);
  radioButtons->show();
  XradioButton->setChecked(true);
  YradioButton->setChecked(false);
  ZradioButton->setChecked(false);
  // Annotate the image with window/level and mouse over pixel
  // information
  vtkSmartPointer<vtkCornerAnnotation> cornerAnnotation =
    vtkSmartPointer<vtkCornerAnnotation>::New();
  cornerAnnotation->SetLinearFontScaleFactor(2);
  cornerAnnotation->SetNonlinearFontScaleFactor(1);
  cornerAnnotation->SetMaximumFontSize(10);
  cornerAnnotation->SetText(0, "<image_and_max>\n<window>\n<level>" ) ;
  cornerAnnotation->GetTextProperty()->SetColor(0, 1, 0);
  image_view->GetRenderer()->AddViewProp(cornerAnnotation);
  /*planeWidget->SetInteractor(visu->GetRenderWindow()->GetInteractor());
  double origin[3] = {0, 0,0};
  double point1[3] = {0, 10,0};
  double point2[3] = {30, 0,0};
  planeWidget->SetOrigin(origin);
  planeWidget->SetPoint1(point1);
  planeWidget->SetPoint2(point2);
  planeWidget->UpdatePlacement();
  planeWidget->UseContinuousCursorOn();
  std::cout<<planeWidget->GetCursorProperty()<<std::endl;*/
  //planeWidget->On();
  visu->SetRenderWindow(image_view->GetRenderWindow());
  image_view->SetupInteractor(visu->GetRenderWindow()->GetInteractor());
  slotSliceOrientation(0);
  image_view->GetRenderer()->ResetCamera();
  image_view->Render();
  visu->update() ;
  this->setWindowTitle( QString(m_FileName.c_str() ) ) ;
  windowLevelGroup->show();
  sliceScroll->show();
  interpolateCheckBox->show();
  emit resetWindowLevel() ;
}

void TestQt4::dragEnterEvent(QDragEnterEvent *Qevent)
{
    Qevent->accept();
}


void TestQt4::dropEvent(QDropEvent* Qevent)
{
    const QMimeData* mimeData = Qevent->mimeData();
    if( mimeData->hasUrls() )
    {
        QList <QUrl> urlList = mimeData->urls();
        QMessageBox msgBox;
        if( urlList.size() > 1 )
        {
           msgBox.setText("Select only one file");
           msgBox.exec();
           return ;
        }
        QString filePath = urlList.at(0).toLocalFile();
        //Checks extension:
        bool extensionIsCorrect = false ;
        for( size_t i = 0 ; i < m_SupportedExtensions.size() ; i++ )
        {
          if( filePath.endsWith( std::string("."+m_SupportedExtensions[i]).c_str() ) )
          {
            extensionIsCorrect = true ;
            break ;
          }
        }
        if( !extensionIsCorrect )
        {
           msgBox.setText("Extension not supported");
           msgBox.exec();
           return ;
        }
        if( QFileInfo( filePath ).exists() )
        {
           m_FileName = filePath.toStdString() ;
           FileChanged() ;
        }
    }
}
