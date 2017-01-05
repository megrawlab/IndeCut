#include<iostream>
#include<fstream>
using namespace std;

void quick(int,int,int,int[],int[]);

void ordina(int v[],int pos[],int n){
int sx,dx,m;

//cout<<"dimensione "<<endl;
//cin>>n;
/*int v[n],pos[n];
ifstream f("vettore.dat");
for(i=0;i<n;i++){
 f>>v[i];
 pos[i]=i;
}*/

sx=0;
dx=n-1;
m=v[int(n/2.)];

quick(sx,dx,m,v,pos);
/*for(i=0;i<n;i++){
cout<<v[i]<<endl;
cout<<"posizioni"<<endl;
cout<<pos[i]<<endl;
}
return 0;*/
 
}
