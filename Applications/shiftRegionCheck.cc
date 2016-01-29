#include <irtkImage.h>
#include <irtkTransformation.h>

void usage(char *command)
{
  cerr << "Usage: " << command << " input metadata segmentation output\n" << endl;
  cerr << "Transform the input image using the given vector field." << endl;
  cerr << endl;
  exit(1);
}

int main(int argc, char *argv[])
{
  char *command = argv[0];
  argc--; argv++;
  if (argc < 3){ usage(command); }

  // Filename
  char *input_name = argv[0];
  argc--; argv++;
  char *metadata_name = argv[0];
  argc--; argv++;
  char *segmentation_name = argv[0];
  argc--; argv++;
  char *output_name = argv[0];
  argc--; argv++;

  // Read images
  // irtkGreyImage: short data type
  // irtkRealImage: double data type
  irtkGreyImage input(input_name); // 3D image
  irtkGreyImage metadata(metadata_name); // 4D image
  irtkRealImage segmentation(segmentation_name);     // 3D

  // The image attributes
  irtkImageAttributes attr = input.GetImageAttributes();
  int X = attr._x;
  int Y = attr._y;
  int Z = attr._z;

  // Check whether input and vec have the same dimensionality.
  irtkImageAttributes segmentationAttrs = segmentation.GetImageAttributes();
  int segX = segmentationAttrs._x;
  int segY = segmentationAttrs._y;
  int segZ = segmentationAttrs._z;
  if (segX != X || segY != Y || segZ != Z) {
    fprintf(stderr, "Input image and vector field do not have the same dimensionality!\n");
    return 1;
  }

  // Get the worldToImage matrix and the imageToWorld matrix from the metadata file
  irtkMatrix imageToWorldMatrix = metadata.GetImageToWorldMatrix();
  irtkMatrix worldToImageMatrix = metadata.GetWorldToImageMatrix();

  // Set up the image interpolator
  irtkInterpolationMode interp_mode = Interpolation_Linear;
  irtkInterpolateImageFunction *interp = irtkInterpolateImageFunction::New(interp_mode, &input);
  interp->SetInput(&input);
  interp->Initialize();

  // Create the output image
  irtkGreyImage output(attr);
  
  // Translate by 10 mm
  irtkMatrix worldTransform(4, 4);
  worldTransform.Put(0, 0, 1);
  worldTransform.Put(0, 1, 0);
  worldTransform.Put(0, 2, 0);
  worldTransform.Put(0, 3, 0);
  worldTransform.Put(1, 0, 0);
  worldTransform.Put(1, 1, 1);
  worldTransform.Put(1, 2, 0);
  worldTransform.Put(1, 3, 0);
  worldTransform.Put(2, 0, 0);
  worldTransform.Put(2, 1, 0);
  worldTransform.Put(2, 2, 1);
  worldTransform.Put(2, 3, 10);
  worldTransform.Put(3, 0, 0);
  worldTransform.Put(3, 1, 0);
  worldTransform.Put(3, 2, 0);
  worldTransform.Put(3, 3, 1);
  printf("World transform\n");
  worldTransform.Print();
  
  irtkMatrix eye(4, 4);
  eye.Put(0, 0, 1);
  eye.Put(0, 1, 0);
  eye.Put(0, 2, 0);
  eye.Put(0, 3, 0);
  eye.Put(1, 0, 0);
  eye.Put(1, 1, 1);
  eye.Put(1, 2, 0);
  eye.Put(1, 3, 0);
  eye.Put(2, 0, 0);
  eye.Put(2, 1, 0);
  eye.Put(2, 2, 1);
  eye.Put(2, 3, 0);
  eye.Put(3, 0, 0);
  eye.Put(3, 1, 0);
  eye.Put(3, 2, 0);
  eye.Put(3, 3, 1);
  printf("Eye(4)\n");
  eye.Print();
  
  printf("World2Image\n");
  worldToImageMatrix.Print();
  
  printf("Image2WorldMatrix\n");
  imageToWorldMatrix.Print();
  
  // Only last column of transform matrix should have non-zero values
  irtkMatrix transform = worldToImageMatrix * worldTransform * imageToWorldMatrix - eye;
  printf("Transform\n");
  transform.Print();

  // Transform the image
  for (int z = 0; z < Z; z++) {
    for (int y = 0; y < Y; y++) {
      for (int x = 0; x < X; x++) {
        if (segmentation.Get(x, y, z) == 0) {
		  continue;
		}
	    // Voxel coordinate
	    double x2 = x, y2 = y, z2 = z;

	    // Displacement vector
	    // Assuming the 4-th dimension of the vector field image denotes the displacement.
		double u, v, w;
	
		u = transform.Get(0, 0) * x + transform.Get(0, 1) * y + transform.Get(0, 2) * z + transform.Get(0, 3);
		v = transform.Get(1, 0) * x + transform.Get(1, 1) * y + transform.Get(1, 2) * z + transform.Get(1, 3);
		w = transform.Get(2, 0) * x + transform.Get(2, 1) * y + transform.Get(2, 2) * z + transform.Get(2, 3);
	
		x2 += u;
		y2 += v;
		z2 += w;

		// Compute the interpolated voxel intensity
		// irtk/image++/src/irtkLinearInterpolateImageFunction.cc implements the interpolation.
		double val;
		val = interp->Evaluate(x2, y2, z2);

		// Assign to the voxel in the output image
		output.Put(x, y, z, val);
      }
    }
  }

  // Write the output image
  output.Write(output_name);

  return 0;
}
