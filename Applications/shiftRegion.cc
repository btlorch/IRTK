#include <irtkImage.h>
#include <irtkTransformation.h>

void usage(char *command)
{
  cerr << "Usage: " << command << " input vector_field output\n" << endl;
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
  char *vec_name = argv[0];
  argc--; argv++;
  char *output_name = argv[0];
  argc--; argv++;

  // Read images
  // irtkGreyImage: short data type
  // irtkRealImage: double data type
  irtkGreyImage input(input_name); // 3D image
  irtkRealImage vec(vec_name);     // 4D image

  // The image attributes
  irtkImageAttributes attr = input.GetImageAttributes();
  int X = attr._x;
  int Y = attr._y;
  int Z = attr._z;

  // Check whether input and vec have the same dimensionality.
  irtkImageAttributes vectorFieldAttrs = vec.GetImageAttributes();
  int vecX = vectorFieldAttrs._x;
  int vecY = vectorFieldAttrs._y;
  int vecZ = vectorFieldAttrs._z;
  int vecT = vectorFieldAttrs._t;
  if (vecX != X || vecY != Y || vecZ != Z) {
    fprintf(stderr, "Input image and vector field do not have the same dimensionality!\n");
    return 1;
  }
  if (vecT != 3) {
    fprintf(stderr, "The vector field does not define 3D vectors for each voxel.\n");
    return 1;
  }

  // Set up the image interpolator
  irtkInterpolationMode interp_mode = Interpolation_Linear;
  irtkInterpolateImageFunction *interp = irtkInterpolateImageFunction::New(interp_mode, &input);
  interp->SetInput(&input);
  interp->Initialize();

  // Create the output image
  irtkGreyImage output(attr);

  // Transform the image
  for (int z = 0; z < Z; z++) {
    for (int y = 0; y < Y; y++) {
      for (int x = 0; x < X; x++) {
	// Voxel coordinate
	// Depending on the definition of your vector field, you may not need convert into the world coordinates.
	double x2 = x, y2 = y, z2 = z;
	// input.ImageToWorld(x2, y2, z2);

	// Displacement vector
	// Assuming the 4-th dimension of the vector field image denotes the displacement.
	double u, v, w;
	u = vec.Get(x, y, z, 0);
	v = vec.Get(x, y, z, 1);
	w = vec.Get(x, y, z, 2);
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
