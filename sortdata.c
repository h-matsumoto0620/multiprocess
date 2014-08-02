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
    cl_program program = NULL;
    cl_kernel kernel = NULL;
    cl_platform_id platform_id = NULL;
    cl_uint ret_num_devices;
    cl_uint ret_num_platforms;
    cl_int ret;

    int *value;
    int *state;
    char name[128];

    value = (int *)malloc(9*sizeof(int));
    state = (int *)malloc(9*sizeof(int));


    FILE *fp;
    char fileName[] = "./merge.cl";
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


    ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
    ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &ret_num_devices);

    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);

    command_queue = clCreateCommandQueue(context, device_id, 0, &ret);

    vmem = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int), NULL, &ret);
    smem = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int), NULL, &ret);
    nmem = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(char), NULL, &ret);

    program = clCreateProgramWithSource(context, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);

    ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);

    kernel = clCreateKernel(program, "hello", &ret);

    int a = 10000;

    ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&vmem);
    ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&smem);
    ret = clSetKernelArg(kernel, 2, sizeof(unsigned int), (void *)&a);
    ret = clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *)&nmem);

    size_t global_item_size = 9;
    size_t local_item_size = 1;

    ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);

    ret = clEnqueueReadBuffer(command_queue, vmem, CL_TRUE, 0, 9*sizeof(int), value, 0, NULL, NULL);
    ret = clEnqueueReadBuffer(command_queue, smem, CL_TRUE, 0, 9*sizeof(int), state, 0, NULL, NULL);
    ret = clEnqueueReadBuffer(command_queue, nmem, CL_TRUE, 0, sizeof(char), name, 0, NULL, NULL);

    for(int i=0; i<9; i++)
    {
        printf("%s\n", name[i]);
        printf("%d %d\n",value[i],state[i]);
    }


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