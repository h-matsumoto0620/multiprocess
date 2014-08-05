#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#define MAX_SOURCE_SIZE (0x100000)

#define DATA 10000 //データ数
#define KIND 9 //データ種類数
#define DATA_SIZE DATA*KIND

int main()
{
    cl_device_id device_id = NULL;
    cl_context context = NULL;
    cl_command_queue command_queue = NULL;
    cl_mem vmem = NULL;
    cl_mem smem = NULL;
    cl_mem cmem = NULL;
    cl_mem omem = NULL;
    cl_program program = NULL;
    cl_kernel kernel = NULL;
    cl_platform_id platform_id = NULL;
    cl_uint ret_num_devices;
    cl_uint ret_num_platforms;
    cl_int ret;

    //データファイル名
    char* name[] = {
      "potate",
      "chicken",
      "onion",
      "cumin",
      "turmeric",
      "cardamom",
      "coriander",
      "ginger",
      "red_pepper"
    };

    int value[DATA_SIZE];  //価格
    int state[DATA_SIZE];  //状態
    float cost[DATA_SIZE]; //価格/状態
    int result[KIND*2];    //結果
    char datname[64];      //データファイル名
    int i,j;
    int count = 0;
    int dat_size = DATA;

    FILE *fp;
    char fileName[] = "./kernel.cl"; //カーネルファイル名
    char *source_str;
    size_t source_size;

    //カーネルの読み込み
    fp = fopen(fileName, "r");
    if (!fp){
        fprintf(stderr, "Failed to load kernel\n");
        exit(1);
    }

    source_str = (char*)malloc(MAX_SOURCE_SIZE);
    source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
    fclose(fp);

    //データファイルの読み込み
    for(i=0; i<9; i++){

      memset(datname, '\0', 64);
      sprintf(datname,"%s.txt",name[i]);

      if((fp = fopen(datname, "r")) == NULL){
	printf("%s.txt not found.\n", name[i]);
	exit(1);
      }

      while( fscanf( fp, "%d %d", &value[count], &state[count]) != EOF){
	count++;
      }
    }

    fclose(fp);


    //デバイスIDを取得
    ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
    ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &ret_num_devices);

    //コンテキストの作成
    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
    if (!context)
      {
	printf("Error: Failed to create a compute context!\n");
	return EXIT_FAILURE;
      }

    //コマンドキューの作成
    command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
    if (!command_queue)
      {
        printf("Error: Failed to create a command commands!\n");
        return EXIT_FAILURE;
      }

    //カーネル用のバッファの作成
    vmem = clCreateBuffer(context, CL_MEM_READ_WRITE, DATA_SIZE*sizeof(int), NULL, &ret);
    smem = clCreateBuffer(context, CL_MEM_READ_WRITE, DATA_SIZE*sizeof(int), NULL, &ret);
    cmem = clCreateBuffer(context, CL_MEM_READ_WRITE, DATA_SIZE*sizeof(int), NULL, &ret);
    omem = clCreateBuffer(context, CL_MEM_READ_WRITE, KIND*2*sizeof(int), NULL, &ret);
    if (!vmem || !smem || !cmem || !omem)
      {
	printf("Error: Failed to allocate device memory!\n");
	exit(1);
      }

    //カーネル用のバッファにデータをコピーする
    ret = clEnqueueWriteBuffer(command_queue, vmem, CL_TRUE, 0, DATA_SIZE*sizeof(int), value, 0, NULL, NULL);
    ret = clEnqueueWriteBuffer(command_queue, smem, CL_TRUE, 0, DATA_SIZE*sizeof(int), state, 0, NULL, NULL);
    if (ret != CL_SUCCESS)
      {
	printf("Error: Failed to write to source array!\n");
        exit(1);
      }

    //カーネルからプログラムを作成
    program = clCreateProgramWithSource(context, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);
    if (!program)
      {
	printf("Error: Failed to create compute program!\n");
	return EXIT_FAILURE;
      }

    //カーネルプログラムのコンパイル
    ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
    if (ret != CL_SUCCESS)
      {
	size_t len;
	char buffer[2048];

	printf("Error: Failed to build program executable!\n");
	clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
	printf("%s\n", buffer);
	exit(1);
      }

    //カーネルの作成
    kernel = clCreateKernel(program, "mg_sort", &ret);
    if (!kernel || ret != CL_SUCCESS)
      {
	printf("Error: Failed to create compute kernel!\n");
	exit(1);
      }

    //カーネル関数の引数を設定
    ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&vmem);
    ret |= clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&smem);
    ret |= clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&cmem);
    ret |= clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *)&omem);
    ret |= clSetKernelArg(kernel, 4, sizeof(int), (void *)&dat_size);
    if (ret != CL_SUCCESS)
      {
	printf("Error: Failed to set kernel arguments! %d\n", ret);
	exit(1);
      }

    size_t global_item_size = 9;
    size_t local_item_size = 1;

    //kernelでそれぞれ分担して実行
    ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);
    if (ret)
      {
	printf("Error: Failed to execute kernel!\n");
	return EXIT_FAILURE;
      }

    //カーネルから結果を読み込む
    ret = clEnqueueReadBuffer(command_queue, omem, CL_TRUE, 0, KIND*2*sizeof(int), result, 0, NULL, NULL);
    if (ret != CL_SUCCESS)
      {
	printf("Error: Failed to read output array! %d\n", ret);
	exit(1);
      }

    //結果の出力
    for(int i=0; i<9; i++)
      {
	printf("%s\n", name[i]);
	printf("%d %d\n",result[2*i+0],result[2*i+1]);
      }

    //お掃除
    ret = clFlush(command_queue);
    ret = clFinish(command_queue);
    ret = clReleaseKernel(kernel);
    ret = clReleaseProgram(program);
    ret = clReleaseMemObject(vmem);
    ret = clReleaseMemObject(smem);
    ret = clReleaseCommandQueue(command_queue);
    ret = clReleaseContext(context);

    free(source_str);
}
