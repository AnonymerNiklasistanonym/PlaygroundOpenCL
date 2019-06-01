int compute_factorial(int n)
{
	int factorial = 1;

	for (int i = 1; i <= n; i++)
	{
		factorial *= i;
	}
	return factorial;
}

__kernel void kernelSimple (__global int* device_output, const unsigned int factorial) {
	const unsigned int x = get_global_id(0);
	const unsigned int y = get_global_id(1);
	const unsigned int x_size = get_global_size(0);
	const unsigned int y_size = get_global_size(1);
	const unsigned int currentPixel = x + y * x_size;
	device_output[currentPixel] = currentPixel + compute_factorial(factorial);
}