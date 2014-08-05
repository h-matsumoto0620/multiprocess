#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA 10000 //ファイルのデータ数
#define KIND 9 //データの種類数

typedef struct{
  int value; //価格
  int state; //状態
  int cost;  //value/state
} food;

void MergeSort(food x[], int left, int right);

int main(){

  FILE *fp;
  int count;
  int i,j;
  char filename[32];

  food foods[DATA];

  //商品名
  char* name[KIND] = {"potate",
		      "chicken",
		      "onion",
		      "cumin",
		      "turmeric",
		      "cardamom",
		      "coriander",
		      "ginger",
		      "red_pepper"
  };


  for(j=0; j<KIND; j++){

    memset( filename, 0, sizeof(char) );
    count = 0;

    sprintf(filename, "./%s.txt",name[j]);

    if((fp = fopen(filename, "r")) == NULL){
      printf("File not found.\n");
      exit(1);
    }

  //ファイルからデータを構造体に格納
    while( fscanf( fp, "%d %d", &foods[count].value, &foods[count].state) != EOF){
      count++;
    }

    //価値を計算(並列にしたい)
    for(i = 0; i<DATA; i++){
      foods[i].cost = foods[i].value/foods[i].state;
    }

    //マージソート(並列にしたい)
    MergeSort(foods, 0, DATA-1);

    printf("%s\n",name[j]);
    printf( "%d %d\n", foods[DATA-1].value, foods[DATA-1].state);
  }

  fclose(fp);
  return 0;
}

void MergeSort(food x[], int left, int right){
  int mid, i, j, k;
  food temp[DATA];

  if (left >= right)              /* 配列の要素がひとつなら */
    return;                     /* 何もしないで戻る */

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
