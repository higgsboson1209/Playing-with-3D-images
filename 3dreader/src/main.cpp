#include <vtkCamera.h>
#include<iostream>
#include <vtkColorTransferFunction.h>
#include <vtkFixedPointVolumeRayCastMapper.h>
#include <vtkNrrdReader.h>
#include <vtkNamedColors.h>
#include <vtkPiecewiseFunction.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <array>
#include<vtkOpenGLGPUVolumeRayCastMapper.h>
void setcolors(vtkSmartPointer<vtkNamedColors> colors)
{
    std::array<unsigned char, 4> bkg{ {255, 255, 255, 255} };
    colors->SetColor("BkgColor", bkg.data());
}
void readfile(vtkSmartPointer<vtkNrrdReader> reader, std::string filename)
{
    reader->SetFileName(filename.c_str());
}
void setrenderer(vtkSmartPointer<vtkRenderer> ren, vtkSmartPointer<vtkRenderWindow> renWin)
{
    renWin->AddRenderer(ren);
}
void setinteractor(vtkSmartPointer<vtkRenderWindow> renWin, vtkSmartPointer<vtkRenderWindowInteractor> iren)
{
    iren->SetRenderWindow(renWin);
}
void setmapper(vtkSmartPointer<vtkOpenGLGPUVolumeRayCastMapper> volumeMapper, vtkSmartPointer<vtkNrrdReader> reader)
{
    volumeMapper->SetInputConnection(reader->GetOutputPort());
}
void setvolumecolor(vtkSmartPointer<vtkColorTransferFunction>volumeColor)
{
    volumeColor->AddRGBPoint(0, 0.0, 0.0, 0.0);
    volumeColor->AddRGBPoint(500, 1.0, 0.5, 0.3);
    volumeColor->AddRGBPoint(1000, 1.0, 0.5, 0.3);
    volumeColor->AddRGBPoint(1150, 1.0, 1.0, 0.9);
}
void setvolumescalaropacity(vtkSmartPointer<vtkPiecewiseFunction> volumeScalarOpacity)
{
    //changes oppacity for different tissues
    volumeScalarOpacity->AddPoint(0, 0.00);
    volumeScalarOpacity->AddPoint(500, 0.2);
    volumeScalarOpacity->AddPoint(1000, 0.3);
    volumeScalarOpacity->AddPoint(1150, 1);
}
void setvolumegradientopacity(vtkSmartPointer<vtkPiecewiseFunction> volumeGradientOpacity)
{
    volumeGradientOpacity->AddPoint(0, 0.0);
    volumeGradientOpacity->AddPoint(90, 0.5);
    volumeGradientOpacity->AddPoint(100, 1.0);

}
void setvolumeproperty(vtkSmartPointer<vtkVolumeProperty> volumeProperty, vtkSmartPointer<vtkPiecewiseFunction> volumeGradientOpacity, vtkSmartPointer<vtkPiecewiseFunction> volumeScalarOpacity, vtkSmartPointer<vtkColorTransferFunction>volumeColor)
{
    volumeProperty->SetInterpolationTypeToLinear();
    volumeProperty->ShadeOn();
    volumeProperty->SetGradientOpacity(volumeGradientOpacity);
    volumeProperty->SetAmbient(0.4);
    volumeProperty->SetDiffuse(0.6);
    volumeProperty->SetSpecular(0.2);
}
void setcamera(vtkCamera* camera, vtkSmartPointer<vtkVolume> volume)
{
    double* c = volume->GetCenter();
    camera->SetViewUp(0, 0, -1);
    camera->SetPosition(c[0], c[1] - 400, c[2]);
    camera->SetFocalPoint(c[0], c[1], c[2]);
    camera->Azimuth(30.0);
    camera->Elevation(30.0);
}
int main(int argc, char* argv[])
{
    
    std::string filename = "C:\\Users\\saksh\\Downloads\\kingsnake.nhdr";
    vtkSmartPointer<vtkNamedColors> colors =
        vtkSmartPointer<vtkNamedColors>::New();
    setcolors(colors);
    

    // Create the renderer, the render window, and the interactor. The renderer
    // draws into the render window, the interactor enables mouse- and
    // keyboard-based interaction with the scene.
    vtkSmartPointer<vtkRenderer> ren =
        vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderWindow> renWin =
        vtkSmartPointer<vtkRenderWindow>::New();
    setrenderer(ren, renWin);
    vtkSmartPointer<vtkRenderWindowInteractor> iren =
        vtkSmartPointer<vtkRenderWindowInteractor>::New();
    setinteractor(renWin, iren);

   //used to read Nrrd files
    vtkSmartPointer<vtkNrrdReader> reader =
        vtkSmartPointer<vtkNrrdReader>::New();
    readfile(reader, filename);

    // The volume will be displayed by ray-cast alpha compositing.
    // A ray-cast mapper is needed to do the ray-casting.
    vtkSmartPointer<vtkOpenGLGPUVolumeRayCastMapper> volumeMapper =
        vtkSmartPointer<vtkOpenGLGPUVolumeRayCastMapper>::New();
    setmapper(volumeMapper, reader);
    vtkSmartPointer<vtkColorTransferFunction>volumeColor =
        vtkSmartPointer<vtkColorTransferFunction>::New();
    setvolumecolor(volumeColor);

    vtkSmartPointer<vtkPiecewiseFunction> volumeScalarOpacity =
        vtkSmartPointer<vtkPiecewiseFunction>::New();
    setvolumescalaropacity(volumeScalarOpacity);
    // The gradient opacity function is used to decrease the opacity
    // in the "flat" regions of the volume while maintaining the opacity
    // at the boundaries between tissue types.  The gradient is measured
    // as the amount by which the intensity changes over unit distance.
    // For most medical data, the unit distance is 1mm.
    vtkSmartPointer<vtkPiecewiseFunction> volumeGradientOpacity =
        vtkSmartPointer<vtkPiecewiseFunction>::New();
    
    setvolumegradientopacity(volumeGradientOpacity);
    
    vtkSmartPointer<vtkVolumeProperty> volumeProperty =
        vtkSmartPointer<vtkVolumeProperty>::New();

    setvolumeproperty(volumeProperty, volumeGradientOpacity, volumeScalarOpacity, volumeColor);

   
    // The vtkVolume is a vtkProp3D (like a vtkActor) and controls the position
    // and orientation of the volume in world coordinates.
    vtkSmartPointer<vtkVolume> volume =
        vtkSmartPointer<vtkVolume>::New();
    volume->SetMapper(volumeMapper);
    volume->SetProperty(volumeProperty);

    // Finally, add the volume to the renderer
    ren->AddViewProp(volume);

    // Set up an initial view of the volume.  The focal point will be the
    // center of the volume, and the camera position will be 400mm to the
    // patient's left (which is our right).
    vtkCamera* camera = ren->GetActiveCamera();
    setcamera(camera,volume);

    // Set a background color for the renderer
    ren->SetBackground(colors->GetColor3d("BkgColor").GetData());

    // Increase the size of the render window
    renWin->SetSize(640, 480);

    // Interact with the data.
    renWin->Render();
    iren->Start();

    return EXIT_SUCCESS;
}