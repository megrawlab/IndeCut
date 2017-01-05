void scambia(int&,int&);
void scambiabool(bool&,bool&);

void quickbool(int sx,int dx,int m,int v[],bool pos[]){
int i,j;
i=sx;
j=dx;
do{
 while(v[i]>m) i++;
 while(v[j]<m) j--;
 if(i<=j){
  scambia(v[i],v[j]);
  scambiabool(pos[i],pos[j]);
  i++;
  j--;
 }
} while(j>=i);
if(sx<j){
 m=v[int((sx+j)/2.)];
 quickbool(sx,j,m ,v,pos);
}
if(dx>i){
m=v[int((i+dx)/2.)];
quickbool(i,dx,m,v,pos);
}
}  

