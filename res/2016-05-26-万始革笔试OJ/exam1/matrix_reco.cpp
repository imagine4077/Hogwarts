/**
 * AUTHOR: Zhao Wei
 * SCHOOL: Nanjing Universicy
 * DATE: 2016-05-25
 * **/
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <set>
#include <cstdio>
#include <vector>
#include <utility>
#include <cmath>

/** to check the details **/
//#define debug_detail
/** to check the process **/
//#define debug
//#define show_split_point

using namespace std;

void print_matrix(int** matrix, int row,int col){
	for(int i=0;i<row;i++){
		for(int j=0;j<col;j++) printf("%d ",matrix[i][j]);
		cout << endl;
	}
}

void _clear(vector<int**> vec){
	for(unsigned int i=0;i<vec.size();i++) delete vec[i];
}

bool judge_equal_matrix(vector<int**> vec,int row,int col){
	for(unsigned int ii=0;ii<vec.size()-1;ii++)
		for(unsigned int jj=ii+1;jj<vec.size();jj++){
			int flag=0;
			for(int i=0;i<row;i++)
				for(int j=0;j<col;j++)
					if(vec[ii][i][j]==vec[jj][i][j]) flag++;
			if(flag>=row*col) return true;
		}
	return false;
}

int** matrix_add(int** a, int** b, int row, int col){
	int** ret = new int* [row];
	for(int i=0;i<row;i++){
		ret[i] = new int [col];
		for(int j=0;j<col;j++)
			ret[i][j] = a[i][j]+b[i][j];
	}
	return ret;
}

int** get_zeros_matrix(int row, int col){
	int** ret = new int* [row];
	for(int i=0;i<row;i++){
		ret[i] = new int [col];
		for(int j=0;j<col;j++) ret[i][j]=0;
	}
	return ret;
}

vector<pair<int,int> > get_max_split(int** mat, int row, int col, int k){
	vector<pair<int,int> > ret;
	if(k>4){
		int standar1 = ceil(k/2.0),standar2=floor(k/2.0),mins=k;
		for(int i=0;i<row;i++){
			for(int j=0;j<col;j++){
				int dif = min(abs(mat[i][j]-standar1),abs(mat[i][j]-standar2));
				if(dif<mins) mins = dif;
			}
		}
		mins++;
		for(int i=0;i<row;i++){
			for(int j=0;j<col;j++){
				int dif = min(abs(mat[i][j]-standar1),abs(mat[i][j]-standar2));
				if(dif<=mins && dif!=0 && dif!=k){
					pair<int,int> apair(i,j);
					ret.push_back(apair);
				}
			}
		}
	}
	else{
		int standar1 = ceil(k/2.0),standar2=floor(k/2.0),mins=k;
		for(int i=0;i<row;i++){
			for(int j=0;j<col;j++){
				int dif = min(abs(mat[i][j]-standar1),abs(mat[i][j]-standar2));
				if(dif<mins) mins = dif;
			}
		}
		for(int i=0;i<row;i++){
			for(int j=0;j<col;j++){
				int dif = min(abs(mat[i][j]-standar1),abs(mat[i][j]-standar2));
				if(dif==mins){
					pair<int,int> apair(i,j);
					ret.push_back(apair);
				}
			}
		}
			
	}
	return ret;
}

pair<vector<int**>,vector<int**> > split(vector<int**> vec, pair<int,int> p){
	pair<vector<int**>,vector<int**> > ret;
	for(unsigned int i=0;i<vec.size();i++)
		if(vec[i][p.first][p.second]==0) ret.first.push_back(vec[i]);
		else ret.second.push_back(vec[i]);
	return ret;
}

void print_set(set<pair<int,int> > s){
	for(set<pair<int,int> >::iterator iter=s.begin();iter!=s.end();iter++)
		printf("<%d,%d>\n",(*iter).first,(*iter).second);
}

set<pair<int,int> > solution(vector<int**> vec, int row, int col,set<pair<int,int> > s){
	set<pair<int,int> > ret_set = s;
#ifdef debug
	for(unsigned int i=0;i<vec.size();i++) {cout<<"begin ori matrix "<<i<<":\n";print_matrix(vec[i],row,col);}
#endif
	if(vec.size()<=1) {
#ifdef debug
		cout << "~~~~~~~~~\nONLY ONE\n~~~~~~~~~\n";
#endif
		return ret_set;
	}
	if(judge_equal_matrix(vec,row,col)){
		cout << "INPUT ERROR: has the same pattern"<< endl;
		_clear(vec);
		exit(1);
	}
	int** tmp_sum = get_zeros_matrix(row,col);
	for(unsigned int i=0;i<vec.size();i++){
		int** tmp = tmp_sum;
		tmp_sum = matrix_add(tmp,vec[i],row,col);
		delete tmp;
	}
#ifdef debug_detail
	cout << "sum of matrix:"<<endl;
	print_matrix(tmp_sum,row,col);
#endif
	vector<pair<int,int> > critical_ele = 
		get_max_split(tmp_sum,row,col,vec.size());
#ifdef debug_detail
	for(unsigned int i=0;i<critical_ele.size();i++)
		printf("critical_ele:%d,%d\n",critical_ele[i].first,critical_ele[i].second);
#endif
	/** consider every split and culculate **/
	unsigned int min_px = row*col+1,id=0;
	set<pair<int,int> > min_set;
	for(unsigned int i=0;i<critical_ele.size();i++){
		set<pair<int,int> > tmp_set = s;
		tmp_set.insert(critical_ele[i]);
		pair<vector<int**>,vector<int**> > spl = split(vec,critical_ele[i]);
#ifdef debug_detail
		cout <<"~~~~~~~~~~~\n";
		printf("split point:<%d,%d>\n",critical_ele[i].first,critical_ele[i].second);
		for(unsigned int a1=0;a1<spl.first.size();a1++){
			cout << "print split 1:\n";
			print_matrix(spl.first[a1],row,col);
		}
		for(unsigned int a1=0;a1<spl.second.size();a1++){
			   cout << "print split 2:\n";
			   print_matrix(spl.second[a1],row,col);
		}
		cout <<"~~~~~~~~~~~\n";
#endif
		set<pair<int,int> > px1 = solution(spl.first,row,col,tmp_set);
		set<pair<int,int> >  px2 = solution(spl.second,row,col,tmp_set);
		set<pair<int,int> > union_set;
		set_union(px1.begin(),px1.end(),px2.begin(),px2.end(),inserter(union_set,union_set.begin()));
		union_set.insert(critical_ele[i]);
#ifdef debug
		cout <<"union_set.size():"<<union_set.size()<<"\tmin_px:"<< min_px<<endl;
		cout << "this set: \n";
		print_set(union_set);
		cout << "min_set: \n";
		print_set(min_set);
#endif
		if(union_set.size()<=min_px){
#ifdef debug
			cout <<"++++++++++\n";
			cout << "enter if(union_set.size()<min_px)"<<endl;
			for(unsigned int ij=0;ij<vec.size();ij++) {cout<<"ori matrix "<<ij<<":\n";print_matrix(vec[ij],row,col);}
			cout <<"++++++++++\n";
#endif
			min_set = union_set;
			min_px = union_set.size();
			id = i;
			if(min_px<=ceil(log(vec.size())/log(2))) break;
		}
	}
	set_union(s.begin(),s.end(),min_set.begin(),min_set.end(),inserter(ret_set,ret_set.begin()));
	ret_set.insert(critical_ele[id]);
	delete tmp_sum;
#ifdef debug
	cout << "print ret_set:\n";
	for(unsigned int ij=0;ij<vec.size();ij++) {cout<<"end-ori matrix "<<ij<<":\n";print_matrix(vec[ij],row,col);}
	cout << "print ret_set:\n";
	print_set(ret_set);
#endif
	return ret_set;
}

int main(){
	freopen("in.txt","r",stdin);
	freopen("out.txt","w",stdout);
	int nrow,mcol,k;
	while(EOF!=scanf("%d %d %d",&nrow,&mcol,&k)){
#ifdef debug
	printf("%d %d %d:\n",nrow,mcol,k);
#endif
	set<pair<int,int> > ret;
	vector<int**> vec;
	set<pair<int,int> > s;
	/** get data **/
	while(k--){
		int** matrix = new int* [nrow];
		for(int i=0;i<nrow;i++){
			matrix[i] = new int [mcol];
			for(int j=0;j<mcol;j++) scanf("%1d",&matrix[i][j]);
		}
		vec.push_back(matrix);
	}

#ifdef debug
	for(unsigned int i=0;i<vec.size();i++) {print_matrix(vec[i],nrow,mcol);}
#endif
	ret = solution(vec,nrow,mcol,s);
	cout << ret.size() <<endl;
	/** mission complete **/
	_clear(vec);
#ifdef show_split_point
	print_set(ret);
	cout << "===================="<<endl;
#endif
	}
	return 0;
}

