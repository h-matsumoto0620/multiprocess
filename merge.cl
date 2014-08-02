typedef struct
{
  int value; //価格
  int state; //状態
  float cost; //
} food;

void MergeSort(food x[], int left, int right);

__kernel void poteto(__global int* value, __global int* state, const unsigned int data __global char* name)
{
	int j = get_global_id(0);
	char filename[32];
	DATA = data;
	food foods[DATA];


	char* name[128] = {"potate",
		      "chicken",
		      "onion",
		      "cumin",
		      "turmeric",
		      "cardamom",
		      "coriander",
		      "ginger",
		      "red_pepper"
  };

	//memset( filename, 0, sizeof(char) );
    count = 0;

    sprintf(filename, "./%s.txt",name[j]);

    if((fp = fopen(filename, "r")) == NULL){
      printf("File not found.\n");
      exit(1);
    }

    while( fscanf( fp, "%d %d", &foods[count].value, &foods[count].state) != EOF){
      count++;
    }

    //価値を計算(並列にしたい)
    for(i = 0; i<DATA; i++){
      foods[i].cost = 1.0*foods[i].value/foods[i].state;
    }

    //マージソート(並列にしたい)
    MergeSort(foods, 0, DATA-1);

    //printf("%s\n",name[j]);
    //printf( "%d %d\n", foods[DATA-1].value, foods[DATA-1].state);

    value = foods[DATA-1].value;
    state = foods[DATA-1].state;

  fclose(fp);
}

void MergeSort(food x[], int left, int right)
{
	int mid, i, j, k;
	food temp[DATA];

	if (left >= right)              /* 配列の要素がひとつなら */
    exit(0);                   /* 何もしないで戻る */

  mid = (left + right) / 2;

  MergeSort(x, left, mid);
  MergeSort(x, mid + 1, right);

  for (i = left; i <= mid; i++){
    temp[i].value = x[i].value;
    temp[i].state = x[i].state;
    temp[i].cost  = x[i].cost;
	}

  for (i = mid + 1, j = right; i <= right; i++, j--){
    temp[i].value = x[j].value;
    temp[i].state = x[j].state;
    temp[i].cost  = x[j].cost;
	}

  i = left;
  j = right;

  for (k = left; k <= right; k++){

  	if (temp[i].cost <= temp[j].cost){
      x[k].value = temp[i].value;
      x[k].state = temp[i].state;
      x[k].cost  = temp[i++].cost;
  }

  else{
      x[k].value = temp[j].value;
      x[k].state = temp[j].state;
      x[k].cost  = temp[j--].cost;
  }

	}
}
