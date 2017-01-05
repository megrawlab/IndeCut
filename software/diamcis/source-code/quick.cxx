void scambia(int&,int&);

void quick(int sx,int dx,int m,int v[],int pos[]){
int i,j;
i=sx;
j=dx;
do{
 while(v[i]>m) i++;
 while(v[j]<m) j--;
 if(i<=j){
  scambia(v[i],v[j]);
  scambia(pos[i],pos[j]);
  i++;
  j--;
 }
} while(j>=i);
if(sx<j){
 m=v[int((sx+j)/2.)];
 quick(sx,j,m ,v,pos);
}
if(dx>i){
m=v[int((i+dx)/2.)];
quick(i,dx,m,v,pos);
}
}  

