#include <vector>
#include <iostream>
#include <cstdio>
#include <cstdlib>

using namespace std;

void max_subarr(vector<unsigned int> vec, unsigned int xor_sum,
		bool one_before,unsigned int idx);

unsigned int max_xor;
int main(){
	freopen("in.txt","r",stdin);
	freopen("out.txt","w",stdout);
	int n;
	while(EOF!=scanf("%d",&n)){
		vector<unsigned int> vec;
		unsigned int xor_sum = 0;
		while(n--){
			unsigned int num;
			scanf("%d",&num);
			xor_sum = xor_sum^num;
			vec.push_back(num);
		}
		max_xor = xor_sum;
		max_subarr(vec,xor_sum,false,0);
		cout << max_xor << endl;
	}
}

void max_subarr(vector<unsigned int> vec, unsigned int xor_sum,
		bool one_before, unsigned int cur_idx){
	if(cur_idx>=vec.size()) return;
	unsigned int add_cur = xor_sum^vec[cur_idx];
	max_xor = max_xor>add_cur?max_xor:add_cur;
	if(one_before){
		max_subarr(vec,add_cur,true,cur_idx+1);
	}
	else{
		max_subarr(vec,xor_sum,false,cur_idx+1);
		max_subarr(vec,add_cur,true,cur_idx+1);
	}
	return;
}
