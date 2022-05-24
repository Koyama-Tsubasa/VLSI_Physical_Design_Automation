#include<iostream>
#include<fstream>
#include<string>
#include<map>
#include<vector>
#include<algorithm>
#include<ctime>
#include<chrono>
using namespace std;


//???? Cell structure 
class Cell {
	public:
		int cell_id;
		int cell_weight;
		int pins = 0;
		vector<int> net_in_this_cell;
};

//???? net structure
class Net {
	public:
		int net_id;
		int A_set = 0;
		int B_set = 0;
		int net_weight = 0;
		vector<int> cell_in_this_net;
}; 

//???d balance ??? 
int check_balance(int a , int all) {
	
	if ( abs(2*a-all)>=float(all)/10 ) return -1;
	//if ( a<2.2 or a>12.2 ) return -1; //for practice
	
}

//?p??cut size
int CUT_SIZE(vector<Net> n) {
	
	int cs = 0;
	for (int i=0;i<n.size();i++)
		if ( n[i].A_set!=0 and n[i].B_set!=0 ) cs++;
		
	return cs;
	
} 

int main(int argc , char *argv[]) {
	
	int i,j,k,cid,nid,flag = 0;
	string read_data; //?? data ?? 
	int total_weight = 0;
	int max_pin_num = 0; //max pin num in a cell
	
	int FACM,FBCM; //final A/B cell num
	int difference; //weight difference between A set and B set 
	int final_cutsize = 0; //final cutsize
	int sum = 0; //partial sum
	int max_sum = 0; //maximum partial sum
	map<int,char> result_set; //final cut
	
	ifstream File_C(argv[2]); //read net file
	ifstream File_N(argv[1]); //read cell file
	ofstream output(argv[3]); //set output file
	
	
  auto t_begin = chrono::steady_clock::now();
	/*----------------------------------------------------------------------------------------------------------------*/
	//???? cell ¡N? data structure 

	vector<Cell> cell_by_given; //?s?ªÉ?cell by given order
	map<int,int> cell_in_order; //cell id to given order ?????
	map<int,int> cell_locked; //cell ?O?_ locked 
	
	//????cell ?? data 
	
	int cell_num = 0; //cell ?`???q
	int max_cell_num = 0; //cell max id
	
	while ( getline(File_C,read_data) ) {
		
		Cell cell;
		cell.cell_id = stoi( read_data.substr(read_data.find("c")+1,read_data.find(" ")-1) );
		cell.cell_weight = stoi( read_data.substr(read_data.find(" ")) );
		cell_by_given.push_back(cell);
		cell_in_order[cell.cell_id] = cell_num;
		cell_locked[cell.cell_id] = 0;
		
		cell_num++;
		total_weight += cell.cell_weight;
		
		if ( cell.cell_id > max_cell_num ) max_cell_num = cell.cell_id;
		
	}
	
	File_C.close();
	
	
	//print cell
//	for ( i=0;i<cell_num;i++ ) cout<<"Cell: "<<cell_by_given[i].cell_id<<" "<<cell_by_given[i].cell_weight<<endl;
	
	
	
	/*----------------------------------------------------------------------------------------------------------------*/
	//????net ?? data 
	
	vector<Net> NET;//?s net data 
	int pins = 0 , len , net_num = 0;
	i = 1;
	
	while ( getline(File_N,read_data) ) {
		
		Net N;
		N.net_id = stoi( read_data.substr(read_data.find("n")+1,read_data.find("{")-read_data.find("n")-2) );
		len = read_data.length();
		
		while ( true ) {
			
			next_line:
			
			if ( read_data.find("c")<len ) {
				
				read_data = read_data.substr(read_data.find("c")+1);
				N.cell_in_this_net.push_back( stoi( read_data.substr(0,read_data.find(" ")) ) );
        cid = cell_in_order.find( stoi( read_data.substr(0,read_data.find(" ")) ) )->second;
				N.net_weight += cell_by_given[cid].cell_weight;
				cell_by_given[cid].net_in_this_cell.push_back(i);
				cell_by_given[cid].pins++;
				if ( cell_by_given[cid].pins > max_pin_num ) 
					max_pin_num = cell_by_given[cid].pins;
				
			}
			else if ( read_data.find("}")<len ) break;
			else {
				
				getline(File_N,read_data);
				goto next_line;
				
			}
			
		}
			
		NET.push_back(N);
		net_num++;
		i++;
		
	}
	
	File_N.close();
	//cout<<float(clock())*0.001<<" seconds"<<endl;
	
//	//print net
//	for (i=0;i<NET.size();i++) {
//		
//		cout<<"\nNet: "<<NET[i].net_id<<endl;
//		cout<<NET[i].cell_in_this_net.size()<<endl;
//    cout<<NET[i].net_weight<<endl;
//		for (j=0;j<NET[i].cell_in_this_net.size();j++) cout<<NET[i].cell_in_this_net[j]<<" ";
//		
//	} 
//	cout<<endl;
  auto t_end = chrono::steady_clock::now();
  auto io_time = chrono::duration_cast<chrono::microseconds>(t_end-t_begin).count();	
  
  t_begin = chrono::steady_clock::now();
	/*----------------------------------------------------------------------------------------------------------------*/
	//??cell ¡N??? A,B sets by nets ( ???H net ?øA?[?i A set , ?W?L?`???q¡N@?b???¯oA?ºà?¡NU cell ?[?i B set )
	
	map<int,char> partition_set;
	int A_weight = 0;
	int B_weight = 0;
	int A_cell_num = 0;
	int B_cell_num = 0;
	
	for (i=0;i<net_num;i++) {
		
		if ( flag==0 ) {
			
			for (j=0;j<NET[i].cell_in_this_net.size();j++) {
				
				if ( partition_set.find(NET[i].cell_in_this_net[j])==partition_set.end() ) {
				
            				partition_set[NET[i].cell_in_this_net[j]] = 'A';
					          A_cell_num++;
            				A_weight += cell_by_given[cell_in_order.find(NET[i].cell_in_this_net[j])->second].cell_weight;
            
        			}

				NET[i].A_set++;

				if ( A_weight>(total_weight/2) ) {

					flag = 1;
					for ( k=j+1;k<NET[i].cell_in_this_net.size();j++,k++ )	{

						if ( partition_set.find(NET[i].cell_in_this_net[k])->second=='A' ) NET[i].A_set++;
						else {
							if ( partition_set.find(NET[i].cell_in_this_net[k])==partition_set.end() ) {
								partition_set[cell_by_given[cell_in_order.find(NET[i].cell_in_this_net[k])->second].cell_id] = 'B';
							}
							NET[i].B_set++;
					
						}

					}				
				
				}
				
			}

			
		}
		else {
			
			for (j=0;j<NET[i].cell_in_this_net.size();j++) {
				
				if ( partition_set.find(NET[i].cell_in_this_net[j])->second=='A' ) NET[i].A_set++;
				else {
					if ( partition_set.find(NET[i].cell_in_this_net[j])==partition_set.end() ) {
						partition_set[cell_by_given[cell_in_order.find(NET[i].cell_in_this_net[j])->second].cell_id] = 'B';
					}
					NET[i].B_set++;
					
				}
				
			}
			
		}
		
	}
	
	B_weight = total_weight - A_weight;
	B_cell_num = cell_num - A_cell_num;
//	cout<<float(clock())*0.001<<" seconds"<<endl;

	
	/*----------------------------------------------------------------------------------------------------------------*/
	//?ßN?l gain ??
	//cout<<"gain"<<endl;
	map<int,vector<int>> Blist_A; //A set ?? bucket list 
	map<int,vector<int>> Blist_A_pre; //??pre gain ????
  map<int,vector<int>> Blist_B; //B set ?? bucket list
  map<int,vector<int>> Blist_B_pre; //??pre gain ????
	map<int,vector<int>>::iterator iter;
	vector<int>::iterator found;
	
  //?????U?? bucket list
	for (i=(-1)*max_pin_num;i<=max_pin_num;i++)	{
		
		Blist_A[i];
		Blist_B[i];
		Blist_A_pre[i];
		Blist_B_pre[i];
		
	}
 
  int cell_gain;
	int max_gain_A = (-1)*max_pin_num; //max gain in set A
	int max_gain_B = (-1)*max_pin_num; //max gain in set B
	int F; //from block to cell
	int T; //the to block of cell
	int which_cell;	//?P?O?? cell ?b??¡N@?? set , 0-A , 1-B
	int ALL_gain[max_cell_num]; //?s???? cell ?? gain ??
	int ALL_gain_after[max_cell_num]; //?s???? cell ?þvs??¡N??ÞÕ? gain ??
 
  //gain array ?k 0
	for (i=0;i<max_cell_num;i++) {
		
		ALL_gain[i] = 0;
		ALL_gain_after[i] = 0;
		
	}
//	cout<<float(clock())*0.001<<" seconds"<<endl;
	
	//pre_gain ( 1st: only check for bucket list A )
	for (i=0;i<cell_num;i++) {

		if ( partition_set.find(cell_by_given[i].cell_id)->second=='A' ) which_cell = 0;
		else if ( partition_set.find(cell_by_given[i].cell_id)->second=='B' ) which_cell = 1;
		
   /*--------------------------------------------------------
	   for each net n on the base cell do
	   --------------------------------------------------------*/
		cell_gain = 0;
		for (j=0;j<cell_by_given[i].net_in_this_cell.size();j++) {
			
			F = 0;
			T = 0;
			nid = cell_by_given[i].net_in_this_cell[j];
			
      /*--------------------------------------------------------
	      F <- the front block of cell i
        T <- the to block of cell i
	      --------------------------------------------------------*/
			if ( which_cell==0 ) {
				
				F += NET[nid-1].A_set;
				T += NET[nid-1].B_set;
				
			}
			else {
				
				F += NET[nid-1].B_set;
				T += NET[nid-1].A_set;
				
			}
			
      /*--------------------------------------------------------
	      if F(n) = 1 then g(i) <- g(i) + 1
        if T(n) = 0 then g(i) <- g(i) - 1
	      --------------------------------------------------------*/
			if ( F==1 ) cell_gain++;
			if ( T==0 ) cell_gain--;
			
		}
		
		if ( which_cell==0 ) {
			
			Blist_A_pre.find(cell_gain)->second.push_back(cell_by_given[i].cell_id);
			if ( cell_gain > max_gain_A ) max_gain_A = cell_gain;
			
		}
		
	}	
//	cout<<float(clock())*0.001<<" seconds"<<endl;
	
	//move big gain in A to B
	for (i=max_gain_A;i>-1;i--) {
		if ( Blist_A_pre.find(i)->second.size()!=0 )
			for (j=0;j<Blist_A_pre.find(i)->second.size();j++) {
      
        cid = cell_in_order.find( Blist_A_pre.find(i)->second[j] )->second;
				
				if ( check_balance(A_weight-cell_by_given[cid].cell_weight,total_weight)!=-1 ) {
					
					partition_set.find(Blist_A_pre.find(i)->second[j])->second = 'B';
					for (k=0;k<cell_by_given[cid].net_in_this_cell.size();k++) {
						
						NET[ cell_by_given[cid].net_in_this_cell[k]-1 ].A_set--;
						NET[ cell_by_given[cid].net_in_this_cell[k]-1 ].B_set++;
					
					}
					A_weight -= cell_by_given[cid].cell_weight;
					B_weight += cell_by_given[cid].cell_weight;
          A_cell_num--;
          B_cell_num++;
					
				}
				
			}
      
  }
	max_gain_A = (-1)*max_pin_num;
//	cout<<float(clock())*0.001<<" seconds"<<endl;

	//pre gain ( 2nd: only check for bucket list B )
	for (i=0;i<cell_num;i++) {

		if ( partition_set.find(cell_by_given[i].cell_id)->second=='A' ) which_cell = 0;
		else if ( partition_set.find(cell_by_given[i].cell_id)->second=='B' ) which_cell = 1;
		
   /*--------------------------------------------------------
	   for each net n on the base cell do
	   --------------------------------------------------------*/
		cell_gain = 0;
		for (j=0;j<cell_by_given[i].net_in_this_cell.size();j++) {
			
			F = 0;
			T = 0;
			nid = cell_by_given[i].net_in_this_cell[j];
			
      /*--------------------------------------------------------
	      F <- the front block of cell i
        T <- the to block of cell i
	      --------------------------------------------------------*/
			if ( which_cell==0 ) {
				
				F += NET[nid-1].A_set;
				T += NET[nid-1].B_set;
				
			}
			else {
				
				F += NET[nid-1].B_set;
				T += NET[nid-1].A_set;
				
			}
			
      /*--------------------------------------------------------
	      if F(n) = 1 then g(i) <- g(i) + 1
        if T(n) = 0 then g(i) <- g(i) - 1
	      --------------------------------------------------------*/
			if ( F==1 ) cell_gain++;
			if ( T==0 ) cell_gain--;
			
		}
		
		if ( which_cell==1 ) {
			
			Blist_B_pre.find(cell_gain)->second.push_back(cell_by_given[i].cell_id);
			if ( cell_gain > max_gain_B ) max_gain_B = cell_gain;
			
		}
		
	}	
//	cout<<float(clock())*0.001<<" seconds"<<endl;
	
	//move big gain in B to A
	for (i=max_gain_B;i>-1;i--) {
		if ( Blist_B_pre.find(i)->second.size()!=0 )
			for (j=0;j<Blist_B_pre.find(i)->second.size();j++) {
      
        cid = cell_in_order.find( Blist_B_pre.find(i)->second[j] )->second;
				
				if ( check_balance(A_weight+cell_by_given[cid].cell_weight,total_weight)!=-1 ) {
					
					partition_set.find(Blist_B_pre.find(i)->second[j])->second = 'A';
					for (k=0;k<cell_by_given[cid].net_in_this_cell.size();k++) {
						
						NET[ cell_by_given[cid].net_in_this_cell[k]-1 ].A_set++;
						NET[ cell_by_given[cid].net_in_this_cell[k]-1 ].B_set--;
						
					}
					A_weight += cell_by_given[cid].cell_weight;
					B_weight -= cell_by_given[cid].cell_weight;
          B_cell_num--;
          A_cell_num++;
					
				}
				
			}
      
  }
	max_gain_B = (-1)*max_pin_num;
//	cout<<float(clock())*0.001<<" seconds"<<endl;

	//real gain to use in FM algorithm
	for (i=0;i<cell_num;i++) {
		
		if ( partition_set.find(cell_by_given[i].cell_id)->second=='A' ) which_cell = 0;
		else if ( partition_set.find(cell_by_given[i].cell_id)->second=='B' ) which_cell = 1;
		
   /*--------------------------------------------------------
	   for each net n on the base cell do
	   --------------------------------------------------------*/
		cell_gain = 0;
		for (j=0;j<cell_by_given[i].net_in_this_cell.size();j++) {
			
			F = 0;
			T = 0;
			nid = cell_by_given[i].net_in_this_cell[j];
			
      /*--------------------------------------------------------
	      F <- the front block of cell i
        T <- the to block of cell i
	      --------------------------------------------------------*/
			if ( which_cell==0 ) {
				
				F += NET[nid-1].A_set;
				T += NET[nid-1].B_set;
				
			}
			else {
				
				F += NET[nid-1].B_set;
				T += NET[nid-1].A_set;
				
			}
		
      /*--------------------------------------------------------
	      if F(n) = 1 then g(i) <- g(i) + 1
        if T(n) = 0 then g(i) <- g(i) - 1
	      --------------------------------------------------------*/
			if ( F==1 ) {
				
				cell_gain++;
				ALL_gain[cell_by_given[i].cell_id-1]++;
				ALL_gain_after[cell_by_given[i].cell_id-1]++;
				
			}
			if ( T==0 ) {
				
				cell_gain--;
				ALL_gain[cell_by_given[i].cell_id-1]--;
				ALL_gain_after[cell_by_given[i].cell_id-1]--;
				
			}
			
		}
		
		if ( which_cell==0 ) {
			
			Blist_A.find(cell_gain)->second.push_back(cell_by_given[i].cell_id);
			if ( cell_gain > max_gain_A ) max_gain_A = cell_gain;
			
		}
		else {
			
			Blist_B.find(cell_gain)->second.push_back(cell_by_given[i].cell_id);
			if ( cell_gain > max_gain_B ) max_gain_B = cell_gain;
			
		}
		
	}	
 
	
	/*----------------------------------------------------------------------------------------------------------------*/
	//???d balance ( before FM algorithm )
	
	if ( check_balance(A_weight,total_weight)==(-1) ) {
		
		cout<<"Not balanced"<<endl;
		cout<<"total weight = "<<total_weight<<endl;
		cout<<"A_weight = "<<A_weight<<endl;
		cout<<"B_weight = "<<B_weight<<endl;
		return 0;
		
	}
//	cout<<float(clock())*0.001<<" seconds"<<endl;


  /*----------------------------------------------------------------------------------------------------------------*/ 
  //?s¡N@?}?l?? result ??T , ???K¡N@?}?l?N?O best result
	final_cutsize = CUT_SIZE(NET);
	result_set = partition_set;
	FACM = A_cell_num;
	FBCM = B_cell_num;	
  cout<<"Initial cut size : "<<final_cutsize<<endl;
//	cout<<float(clock())*0.001<<" seconds"<<endl;
	
 
	/*----------------------------------------------------------------------------------------------------------------*/ 
	//FM - algorithm
	//cout<<"FM"<<endl;
	int id , based_cell , moved_weight , dis;
	map<int,int> cell_which_to_update; //?s gain ?????Q???L?? cell
	map<int,int>::iterator update;
	
  //?w?]?????? cell ?]¡N@?????h?[¡N@?? if ?n?h???? cell gain <= 0 ??????????
	for (int i=0;i<cell_num;i++) {
		//cout<<"FM : "<<i+1<<endl;
		again_after_changing_gain:
		
		cell_which_to_update.clear(); //clear cell which to update
		flag = 1;
		
		//??n?????? cell 
		if ( max_gain_A >= max_gain_B ) {
			
			which_cell = 0;
			iter = Blist_A.find(max_gain_A);
			
      //?d????¡N@?? cell in Bucket list's max gain ???X balance ?b?
			for (j=0;j<iter->second.size();j++) 
				if ( check_balance(A_weight-cell_by_given[ cell_in_order.find( iter->second[j] )->second ].cell_weight,
						total_weight)!=-1 ) {
					
					flag = 0;
					break;
					
				}
			
      //if ¡NW???????q?S?Ó¬?h??¡NU¡N@??¡Nj?? gain ????
			if ( flag==1 ) {
				
				max_gain_A--;
				
				while ( Blist_A.find(max_gain_A)->second.size()==0 ) {
					
					max_gain_A--;
					if ( max_gain_A<max_gain_B ) break;
						
				}
				
				goto again_after_changing_gain;
				
			}
			
		}
		else {
			
			which_cell = 1;
			iter = Blist_B.find(max_gain_B);
				
      //?d????¡N@?? cell in Bucket list's max gain ???X balance ?b?
			for (j=0;j<iter->second.size();j++) 
				if ( check_balance(A_weight+cell_by_given[ cell_in_order.find( iter->second[j] )->second ].cell_weight,
					total_weight)!=-1 ) {
					
					flag = 0;
					break;
					
				}
			
      //if ¡NW???????q?S?Ó¬?h??¡NU¡N@??¡Nj?? gain ????
			if ( flag==1 ) {
				
				max_gain_B--;
				
				while ( Blist_B.find(max_gain_B)->second.size()==0 ) {
					
					max_gain_B--;
					if ( max_gain_B<max_gain_A ) break;
						
				}
				
				goto again_after_changing_gain;
				
			}
			
		}
		
    if ( ALL_gain[iter->second[j]-1]<=0 ) break; //if this cell's gain <= 0 then end this program
		else sum += ALL_gain[iter->second[j]-1]; //add to partial sum
		id = j; //?s?o?? cell ?b bucket list gain ?o¡N@?Â×??X?? cell ( 0 ~ ... )
		based_cell = iter->second[id]; //?s?? cell id
		moved_weight = cell_by_given[ cell_in_order.find( iter->second[id] )->second ].cell_weight; //?s?? cell weight
//		cout<<"choose cell :"<<float(clock())*0.001<<" seconds"<<endl;
		
		//Update cell gains
		/*--------------------------------------------------------
		  for each net n on the base cell do
		  --------------------------------------------------------*/
		for (j=0;j<cell_by_given[cell_in_order.find(based_cell)->second].net_in_this_cell.size();j++) {
			
			F = 0;
			T = 0;
			nid = cell_by_given[cell_in_order.find(based_cell)->second].net_in_this_cell[j];
			
			for (k=0;k<NET[nid-1].cell_in_this_net.size();k++)
				if (cell_which_to_update.find( NET[nid-1].cell_in_this_net[k] )!=cell_which_to_update.end()) 
					cell_which_to_update[NET[nid-1].cell_in_this_net[k]] = 0;
			
			/*--------------------------------------------------------
		  	  F <- the Front Block of the base cell
		  	  T <- the To Block of the base cell
		      --------------------------------------------------------*/
      if ( which_cell==0 ) {
		    	
	    	F += NET[nid-1].A_set;
	    	T += NET[nid-1].B_set;
		    	
	    }
	    else {
				
		    F += NET[nid-1].B_set;
	      T += NET[nid-1].A_set; 
				
			}
			
			//before move
			/*--------------------------------------------------------
		  	  if T(n) = 0 then increment gains of all free cells on n
		  	  elseif T(n) = 1 then decrement gain of the only T cell on n, if it is free
		      --------------------------------------------------------*/
      if ( T==0 ) {
		    	
    	  for (k=0;k<NET[nid-1].cell_in_this_net.size();k++) {
		    		
    		  ALL_gain_after[NET[nid-1].cell_in_this_net[k]-1]++;
 		      cell_which_to_update.find( NET[nid-1].cell_in_this_net[k] )->second++;
		    	
        }
		    	
			}
			else if ( T==1 ) {
				
				if ( which_cell==0 ) {
					
					for (k=0;k<NET[nid-1].cell_in_this_net.size();k++) {
		    		
		    			if ( partition_set.find( NET[nid-1].cell_in_this_net[k] )->second=='B' ) {
		    				
		    				ALL_gain_after[NET[nid-1].cell_in_this_net[k]-1]--;
		    				cell_which_to_update.find( NET[nid-1].cell_in_this_net[k] )->second--;
		    				
						  }
			    	
			     }
					
				}
				else {
					
					for (k=0;k<NET[nid-1].cell_in_this_net.size();k++) {
		    		
		    			if ( partition_set.find( NET[nid-1].cell_in_this_net[k] )->second=='A' ) {
		    				
		    				ALL_gain_after[NET[nid-1].cell_in_this_net[k]-1]--;
		    				cell_which_to_update.find( NET[nid-1].cell_in_this_net[k] )->second--;
		    				
						}
			    	
					}
					
				}
				
			}
			
			/*--------------------------------------------------------
		  	  F(n) <- F(n) - 1
		  	  T(n) <- T(n) + 1
	      --------------------------------------------------------*/
      F--;
      T++;
      if ( which_cell==0 ) {
				
				NET[nid-1].A_set--;
				NET[nid-1].B_set++;
				
			}
			else {
				
				NET[nid-1].A_set++;
				NET[nid-1].B_set--;
				
			}
		    
      //after move
      /*--------------------------------------------------------
  	    if F(n) = 0 then decrement gains of all free cells on n
	  	  elseif F(n) = 1 then increment gain of the only F cell on n, if it is free
        --------------------------------------------------------*/
      if ( F==0 ) {
		    	
		    	for (k=0;k<NET[nid-1].cell_in_this_net.size();k++) {
		    		
		    		ALL_gain_after[NET[nid-1].cell_in_this_net[k]-1]--;
		    		cell_which_to_update.find( NET[nid-1].cell_in_this_net[k] )->second--;
		    	
				}
		    	
			}
			else if ( F==1 ) {
				
				if ( which_cell==0 ) {
					
					for (k=0;k<NET[nid-1].cell_in_this_net.size();k++) {
		    		
		    			if ( partition_set.find( NET[nid-1].cell_in_this_net[k] )->second=='A' ) {
		    				
		    				ALL_gain_after[NET[nid-1].cell_in_this_net[k]-1]++;
		    				cell_which_to_update.find( NET[nid-1].cell_in_this_net[k] )->second++;
		    				
						}
						
					}
					
				}
				else {
					
					for (k=0;k<NET[nid-1].cell_in_this_net.size();k++) {
		    		
		    			if ( partition_set.find( NET[nid-1].cell_in_this_net[k] )->second=='B' ) {
		    				
		    				ALL_gain_after[NET[nid-1].cell_in_this_net[k]-1]++;
		    				cell_which_to_update.find( NET[nid-1].cell_in_this_net[k] )->second++;
		    				
						}
			    	
					}
					
				}
				
			}
						
		}
//		cout<<"update :"<<float(clock())*0.001<<" seconds"<<endl;
		
		//Delete base cell from Blist
		if ( which_cell==0 ) 
			Blist_A.find( ALL_gain[based_cell-1] )->second.erase(Blist_A.find( ALL_gain[based_cell-1] )->second.begin()+id);
		else 
			Blist_B.find( ALL_gain[based_cell-1] )->second.erase(Blist_B.find( ALL_gain[based_cell-1] )->second.begin()+id);
		cell_locked.find(based_cell)->second = 1;
		//cout<<"delete :"<<float(clock())*0.001<<" seconds"<<endl;
		
		//Update Blist by new gain
		for (update=cell_which_to_update.begin();update!=cell_which_to_update.end();++update) {
			
			cid = update->first;
			
			if ( ( update->second!=0 ) and ( cell_locked.find(cid)->second!=1 ) ) {
				
				if ( partition_set.find(cid)->second=='A' ) {
						
					found = std::find(Blist_A.find( ALL_gain[cid-1] )->second.begin(),Blist_A.find( ALL_gain[cid-1] )->second.end(), cid);
					dis = found-Blist_A.find( ALL_gain[cid-1] )->second.begin();
					Blist_A.find( ALL_gain[cid-1] )->second.erase( Blist_A.find( ALL_gain[cid-1] )->second.begin()+dis );
					Blist_A.find( ALL_gain_after[cid-1] )->second.push_back(cid);
						
				}
				else {
						
					found = std::find(Blist_B.find( ALL_gain[cid-1] )->second.begin(),Blist_B.find( ALL_gain[cid-1] )->second.end(), cid);
					dis = found-Blist_B.find( ALL_gain[cid-1] )->second.begin();
					Blist_B.find( ALL_gain[cid-1] )->second.erase( Blist_B.find( ALL_gain[cid-1] )->second.begin()+dis );
					Blist_B.find( ALL_gain_after[cid-1] )->second.push_back(cid);
						
				}
        
        ALL_gain[cid-1] = ALL_gain_after[cid-1];
				
			}
			
		}
		//cout<<"blist :"<<float(clock())*0.001<<" seconds"<<endl;
	
		//move base cell
		if ( which_cell==0 ) {
			
			partition_set.find(based_cell)->second = 'B';
			A_cell_num--;
			B_cell_num++;
			B_weight += moved_weight;
			A_weight -= moved_weight;
			
		}
		else {
			
			partition_set.find(based_cell)->second = 'A';
			A_cell_num++;
			B_cell_num--;
			B_weight -= moved_weight;
			A_weight += moved_weight;
			
		}
		///cout<<"move :"<<float(clock())*0.001<<" seconds"<<endl;
		
		//?þvs max gain in A,B set
		max_gain_A = (-1)*max_pin_num;
		max_gain_B = (-1)*max_pin_num;
		for (j=max_pin_num;j>=(-1)*max_pin_num;j--) {
			
			if ( Blist_A.find(j)->second.size()!=0 ) {
				
				max_gain_A = j;
				break;
				
			}
			
		} 	
		for (j=max_pin_num;j>=(-1)*max_pin_num;j--) {
			
			if ( Blist_B.find(j)->second.size()!=0 ) {
				
				max_gain_B = j;
				break;
				
			}
			
		}
		//cout<<"gain :"<<float(clock())*0.001<<" seconds"<<endl;
		
    //if partial sum > current maximum partial sum then update maximum partial sum
		if ( sum>=max_sum ) {
			//cout<<"updating"<<endl;
			if ( sum>max_sum ) {
				
				difference = abs( A_weight-B_weight );
				final_cutsize = CUT_SIZE(NET);
				result_set = partition_set;
				FACM = A_cell_num;
				FBCM = B_cell_num;
				
			}
			else if ( abs( A_weight-B_weight )<difference ) {
				
				difference = abs( A_weight-B_weight );
				final_cutsize = CUT_SIZE(NET);
				result_set = partition_set;
				FACM = A_cell_num;
				FBCM = B_cell_num;
				
			}
			max_sum = sum;
			
		}
		
//		//checking 
//		cout<<"--------------------------\n--------------------------"<<endl;
//		cout<<"Total_weight: "<<total_weight<<endl;
//		cout<<"A_weight: "<<A_weight<<endl;
//		cout<<"B_weight: "<<B_weight<<endl;
//		
//		for (int iii=1;iii<=cell_num;iii++) cout<<iii<<" "<<partition_set.find(iii)->second<<endl;
//		
//		for (int ii=(-1)*max_pin_num;ii<=max_pin_num;ii++) {
//			
//			cout<<"gain"<<ii<<"\n\n";
//			iter = Blist_A.find(ii);cout<<"A"<<endl;
//			if (!iter->second.empty())
//				for (int jj=0;jj<iter->second.size();jj++)
//					cout<<iter->second[jj]<<endl;
//			iter = Blist_B.find(ii);cout<<"B"<<endl;
//			if (!iter->second.empty())
//				for (int jj=0;jj<iter->second.size();jj++)
//					cout<<iter->second[jj]<<endl;
//			
//		}
//		cout<<"MAX gain of A: "<<max_gain_A<<endl;
//		cout<<"MAX gain of B: "<<max_gain_B<<endl;
//		cout<<float(clock())*0.001<<" seconds"<<endl;
	
	}
  t_end = chrono::steady_clock::now();
  auto com_time = chrono::duration_cast<chrono::microseconds>(t_end-t_begin).count();
 
	//output result
	output<<"cut_size "<<final_cutsize<<"\n\n";
	output<<"A "<<FACM<<"\n\n";
	for (i=1;i<=max_cell_num;i++)
		if ( result_set.find(i)->second=='A' ) output<<"c"<<i<<endl;
	output<<"\nB "<<FBCM<<"\n\n";
	for (i=1;i<=max_cell_num;i++)
		if ( result_set.find(i)->second=='B' ) output<<"c"<<i<<endl;
	cout<<"io time : "<<io_time<<" microseconds"<<endl;
  cout<<"computation time : "<<com_time<<" microseconds"<<endl;
	
	return 0;
	
}
