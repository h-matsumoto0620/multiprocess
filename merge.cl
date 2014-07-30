__kernel void square(
   __global float* input,
   __global float* output,
   const char name,const int count)
{
   int i = get_global_id(0);
   if(i < count)
       puts("%s",&name);
}