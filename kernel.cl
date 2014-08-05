void msort(
	   __global int* value,
	   __global int* state,
	   __global int* cost,
	   int len);

//カーネル関数
__kernel void mg_sort(__global int* value,
		      __global int* state,
		      __global int* cost,
		      __global int* output,
		      const int data)
{
  int i;
  int j = get_global_id(0); //実行時のグローバルID

  //価値(value/state)を計算
  for(i = 0; i<data; i++){
    cost[data*j+i] = value[data*j+i]/state[data*j+i];
  }

  //マージソートの実行
  msort(value+data*j, state+data*j, cost+data*j, data);

  //ソートされた結果から一番値の大きい物を取り出す
  output[2*j+0] = value[data*(j+1)-1];
  output[2*j+1] = state[data*(j+1)-1];
}

//マージソート(再帰なし)
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
	v_tmp[n] = cost[i] <= cost[j] ? value[i] : value[j];
	s_tmp[n] = cost[i] <= cost[j] ? state[i] : state[j];
	c_tmp[n++] = cost[i] <= cost[j] ? cost[i++] : cost[j++];
      }
      while ( i < fe ) {
	v_tmp[n] = value[i];
	s_tmp[n] = state[i];
	c_tmp[n++] = cost[i++];
      }
      while ( j < se ) {
	v_tmp[n] = value[j];
	s_tmp[n] = state[j];
	c_tmp[n++] = cost[j++];
      }
      for ( p = seg; p < n; ++p ) {
	value[p] = v_tmp[p];
	state[p] = s_tmp[p];
	cost[p] = c_tmp[p];
      }
    }
  }
}
