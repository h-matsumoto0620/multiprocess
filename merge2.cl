void msort(
      __global int* value,
      __global int* state,
      __global int* cost,
      int len)
{
  int div, seg, fb, fe, sb, se, i, j, n, p;
  int v_tmp[len];
  int s_tmp[len];
  int c_tmp[len];

  for ( div = 1; div <= len; div *= 2 ) {
    for ( seg = 0; seg < len; seg += div*2 ) {
      fb = n = seg;
      sb = fe = min(fb + div, len);
      se = min(sb + div, len);

      i = fb;
      j = sb;
      while ( i < fe && j < se ) {
	c_tmp[n++] = cost[i] <= cost[j] ? cost[i++] : cost[j++];
      }
      while ( i < fe ) {
	c_tmp[n++] = cost[i++];
      }
      while ( j < se ) {
	c_tmp[n++] = cost[j++];
      }
      for ( p = seg; p < n; ++p ) {
	cost[p] = c_tmp[p];
      }
    }
  }
}

__kernel void mg_sort(__global int* value,
		      __global int* state,
		      const int data,
		      __global int* cost,
		      __global int* output)
{
  int i;
  int j = get_global_id(0);

  //価値を計算(並列にしたい)
  for(i = 0; i<data; i++){
    cost[i] = value[i]/state[i];
  }

  //マージソート(並列にしたい)
  msort(value, state, cost, data);

  //output[0] = value[data-1];
  //output[1] = state[data-1];
  //printf("%d %d\n",value[data-1],state[data-1]);
  printf("cost = %d\n",cost[data-1]);
}
