#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <OpenCL/opencl.h>

int main()
{
    cl_platform_id  platform_id;
    cl_device_id  device_id;
    cl_uint ret, num_platforms, num_devices;

    ret=clGetPlatformIDs(1, &platform_id, &num_platforms);
    printf(" number of platforms: %d\n", num_platforms);
    ret=clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &num_devices);
    printf(" number of devices: %d\n", num_devices);
    // create OpenCL context
    cl_context context;
    context=clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
    // create command queue
    cl_command_queue command_queue;
    command_queue=clCreateCommandQueue(context, device_id, 0, &ret);

    FILE *fp;
    char *source_str;
    size_t source_size;
    char filename[] = "./mult_float.cl";
    fp = fopen(filename, "r");

    source_str=(char*)malloc(MAX_SOURCE_SIZE);
    source_size=fread(source_str, 1, MAX_SOURCE_SIZE, fp);
    fclose(fp);
    // create kernel program from source file
    cl_program program;
    program=clCreateProgramWithSource(context, 1, (const char**)&source_str, (const size_t *)&source_size, &ret);
    // build kernel program
    ret=clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
    // create OpenCL kernel
    cl_kernel clmult;
    clmult=clCreateKernel(program, "mult_all", &ret);

    // create memory object on device
      cl_mem  Vmobj = NULL;
      cl_mem  Wmobj = NULL;
      cl_mem  Umobj = NULL;
      Vmobj = clCreateBuffer(context, CL_MEM_READ_WRITE, Nvst*sizeof(float), NULL, &ret);
      Wmobj = clCreateBuffer(context, CL_MEM_READ_WRITE, Nvst*sizeof(float), NULL, &ret);
      Umobj = clCreateBuffer(context, CL_MEM_READ_WRITE, Ndf*Nst*4*sizeof(float), NULL, &ret);

    // write data on device memory buffer
    ret=clEnqueueWriteBuffer(command_queue, Wmobj, CL_TRUE, 0, Nvst*sizeof(float), wf, 0, NULL, NULL);
    ret=clEnqueueWriteBuffer(command_queue, Umobj, CL_TRUE, 0, Ndf*Nst*4*sizeof(float), uf, 0, NULL, NULL);
    // set arguments of kernel program
    ret=clSetKernelArg(clmult, 0, sizeof(cl_mem), (void *)&Vmobj);
    ret=clSetKernelArg(clmult, 1, sizeof(cl_mem), (void *)&Umobj);
    ret=clSetKernelArg(clmult, 2, sizeof(cl_mem), (void *)&Wmobj);
    ret=clSetKernelArg(clmult, 3, sizeof(float), (void *)&CKs2);
    // run kernel code on device
    size_t  global_item_size = Nst;
    size_t  local_item_size = 1;
    ret=clEnqueueNDRangeKernel(command_queue, clmult_all, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);
    // read data from device memory buffer
    ret=clEnqueueReadBuffer(command_queue, Vmobj, CL_TRUE, 0, Nvst*sizeof(float), vf, 0, NULL, NULL);

    // release memory buffer on device
    ret=clReleaseMemObject(Vmobj);
    ret=clReleaseMemObject(Umobj);
    ret=clReleaseMemObject(Wmobj);
    // release OpenCL kernel
    ret=clReleaseKernel(clmult);
    // release OpenCL items
    ret=clFlush(command_queue);
    ret=clFinish(command_queue);
    ret=clReleaseProgram(program);
    ret=clReleaseCommandQueue(command_queue);
    ret=clReleaseContext(context);
}