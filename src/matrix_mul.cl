__kernel void simpleMultiply(
	__global float* outputC,
	int widthA,
	int heightA,
	int widthB,
	int heightB,
	__global float* inputA,
	__global float* inputB
)
{
	//get global position in y direction
	int row = get_global_id(1);
	//get global position in x direction
	int col = get_global_id(0);

	float sum = 0.0f;

	//Calculate result of one element of Matrix C
	for (int i = 0; i < widthA; i++)
	{
		sum += inputA[row * widthA + i] * inputB[i * widthB + col];
	}
	outputC[row * widthB + col] = sum;
}