#include <stdio.h>
#include <stdlib.h>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#define MAX_SOURCE_SIZE (0x100000)

int main()
{
    cl_device_id device_id = NULL;
    cl_context context = NULL;
    cl_command_queue command_queue = NULL;
    cl_mem vmem = NULL;
    cl_mem smem = NULL;
    cl_mem nmem = NULL;
    cl_mem cmem = NULL;
    cl_mem omem = NULL;
    cl_program program = NULL;
    cl_kernel kernel = NULL;
    cl_platform_id platform_id = NULL;
    cl_uint ret_num_devices;
    cl_uint ret_num_platforms;
    cl_int ret;

    int value[10000];
    int state[10000];
    float cost[10000];
    int i;

    FILE *fp;
    char fileName[] = "./merge2.cl";
    char *source_str;
    size_t source_size;

    fp = fopen(fileName, "r");
    if (!fp){
        fprintf(stderr, "Failed to load kernel\n");
        exit(1);
    }

    source_str = (char*)malloc(MAX_SOURCE_SIZE);
    source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
    fclose(fp);

    char filename[] = "./potate.txt";
    if((fp = fopen(filename, "r")) == NULL){
      printf("File not found.\n");
      exit(1);
    }

    int count = 0;
    while( fscanf( fp, "%d %d", &value[count], &state[count]) != EOF){
      count++;
    }

    fclose(fp);

    int gpu = 0;//デバイスのタイプ、1でGPU、0でCPU
    ret = clGetDeviceIDs(NULL, gpu ? CL_DEVICE_TYPE_GPU : CL_DEVICE_TYPE_CPU, 1, &device_id, NULL);
    if (ret != CL_SUCCESS)
      {
	//このエラーが出る時はgpuの値を変えてみよう
        printf("Error: Failed to create a device group!\n");
        return EXIT_FAILURE;
      }


    //ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
    //ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &ret_num_devices);

    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
    if (!context)
      {
        printf("Error: Failed to create a compute context!\n");
        return EXIT_FAILURE;
      }

    command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
    if (!command_queue)
      {
        printf("Error: Failed to create a command commands!\n");
        return EXIT_FAILURE;
      }

    vmem = clCreateBuffer(context, CL_MEM_READ_WRITE, 10000*sizeof(int), NULL, &ret);
    smem = clCreateBuffer(context, CL_MEM_READ_WRITE, 10000*sizeof(int), NULL, &ret);
    //nmem = clCreateBuffer(context, CL_MEM_READ_WRITE, 9*sizeof(char *), NULL, &ret);
    cmem = clCreateBuffer(context, CL_MEM_READ_WRITE, 10000*sizeof(int), NULL, &ret);
    omem = clCreateBuffer(context, CL_MEM_READ_WRITE, 3*sizeof(int), NULL, &ret);
    if (!vmem || !smem || !cmem || !omem)
      {
        printf("Error: Failed to allocate device memory!\n");
        exit(1);
      }



    ret = clEnqueueWriteBuffer(command_queue, vmem, CL_TRUE, 0, 10000*sizeof(int), value, 0, NULL, NULL);
    ret = clEnqueueWriteBuffer(command_queue, smem, CL_TRUE, 0, 10000*sizeof(int), state, 0, NULL, NULL);
    if (ret != CL_SUCCESS)
      {
        printf("Error: Failed to write to source array!\n");
        exit(1);
      }


    program = clCreateProgramWithSource(context, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);
    if (!program)
      {
        printf("Error: Failed to create compute program!\n");
        return EXIT_FAILURE;
      }

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


    kernel = clCreateKernel(program, "mg_sort", &ret);
    if (!kernel || ret != CL_SUCCESS)
      {
        printf("Error: Failed to create compute kernel!\n");
        exit(1);
      }

    int a = 10000;

    ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&vmem);
    ret |= clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&smem);
    ret |= clSetKernelArg(kernel, 2, sizeof(int), (void *)&a);
    //ret = clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *)&nmem);
    ret |= clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *)&cmem);
    ret |= clSetKernelArg(kernel, 4, sizeof(cl_mem), (void *)&omem);
    if (ret != CL_SUCCESS)
      {
        printf("Error: Failed to set kernel arguments! %d\n", ret);
        exit(1);
      }

    size_t global_item_size = 1;
    size_t local_item_size = 1;
    int result[2];

    ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);
    if (ret)
      {
        printf("Error: Failed to execute kernel!\n");
        return EXIT_FAILURE;
      }

    //ret = clEnqueueReadBuffer(command_queue, vmem, CL_TRUE, 0, 10000*sizeof(int), value, 0, NULL, NULL);
    //ret = clEnqueueReadBuffer(command_queue, smem, CL_TRUE, 0, 10000*sizeof(int), state, 0, NULL, NULL);
    ret = clEnqueueReadBuffer(command_queue, omem, CL_TRUE, 0, 3*sizeof(int), result, 0, NULL, NULL);
    if (ret != CL_SUCCESS)
      {
        printf("Error: Failed to read output array! %d\n", ret);
        exit(1);
      }

    /*
    for(int i=0; i<9; i++)
    {
        printf("%s\n", name[i]);
        printf("%d %d\n",value[i],state[i]);
    }
    */

    //printf("%d %d %f\n",value[0],state[0],cost[0]);
    printf("%d %d\n",result[0],result[1]);

    ret = clFlush(command_queue);
    ret = clFinish(command_queue);
    ret = clReleaseKernel(kernel);
    ret = clReleaseProgram(program);
    ret = clReleaseMemObject(vmem);
    ret = clReleaseMemObject(smem);
    ret = clReleaseMemObject(nmem);
    ret = clReleaseCommandQueue(command_queue);
    ret = clReleaseContext(context);

    free(source_str);
}
