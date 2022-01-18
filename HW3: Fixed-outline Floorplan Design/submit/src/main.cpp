#include<iostream>
#include<fstream>
#include<string>
#include<vector>
#include<map>
#include<ctime>
#include<math.h>
#include<stdlib.h>
#include<chrono>
using namespace std;

/*-----     change some conditions to find magic number ( first change the probability in purturbation )_____11/25     -----*/

// hardblocks
class Block {
	
	public:
		int block_id;
		int width;
		int height;
		int x;
		int y;
		int rotate = 0;
		
		Block* parent = NULL;
		Block* l_child = NULL;
		Block* r_child = NULL; 
	
};


// terminals
class Terminal {
	
	public:
		int terminal_id;
		int x;
		int y;
	
};


// nets
class Net {
	
	public:
		vector<int> terminals;
		vector<int> blocks;
	
};


// variables
ifstream read_hardblocks;
ifstream read_nets;
ifstream read_pl;
ofstream output_file;

int i , j , k , flag = 0 , temp , temp2 , fx , fy , sx , sy , cal_average;
int seed = 6 , SA_times , SA_limit , inside_SA = 10;
int t_x , t_y , max_x , max_y;
int block_num , terminal_num , net_num , pin_num;
float dead_space_ratio;
int total_block_area = 0 , floorplanning_area;
float fp_region = 0;
int w_fp , h_fp;
int perturb , rotate , a , b;
float temperature = 100000 , cooling = 0.85;
int wire_length , w_mx , w_my , w_Mx , w_My;
float cost = 0.0 , alpha = 0.5 , beta = (1-alpha);	// alpha-area , beta-wirelength
float average_WL = 0.0 , average_fpA = 0.0;
int pre_x , pre_y;
char c;
string S_temp;
auto t1 = chrono::steady_clock::now();
auto t2 = chrono::steady_clock::now();
auto t3 = chrono::steady_clock::now();
auto construct_data_structure = chrono::duration_cast<chrono::microseconds>(t3-t2).count();
auto initial_floorplan = chrono::duration_cast<chrono::microseconds>(t3-t1).count();
auto SA_part = chrono::duration_cast<chrono::microseconds>(t3-t1).count();

Block* root;
Block* to_x;
Block* to_y;
Block *a_p , *a_l , *a_r;
Block *b_p , *b_l , *b_r; 
Net* nt;

vector<Block> blocks;
vector<Terminal> terminals;
vector<Net> nets;
vector<int> contour;
vector<int> check_initial;

map<int,int> id_blocks;
map<int,int> id_terminals;

// variables which store final results
int final_WL;
float final_cost;
int final_fpA , final_x , final_y;
Block* final_root;
vector<Block> final_blocks;


// store final blocks
void store_result() {
	
	final_WL = wire_length;
	final_fpA = floorplanning_area;
	final_cost = cost;
	final_x = max_x;
	final_y = max_y;
	final_root = root;
	final_blocks.clear();
	for ( i=0;i<block_num;i++ ) final_blocks.push_back(blocks[i]);
	
}


// restore the best result
void restore_result() {
	
	wire_length = final_WL;
	floorplanning_area = final_fpA;
	cost = final_cost;
	max_x = final_x;
	max_y = final_y;
	root = final_root;
	blocks.clear();
	for ( i=0;i<block_num;i++ ) blocks.push_back(final_blocks[i]);
	
}


// read files
void read_files( char *argv[] ) {
	
//	cout << "read file" << endl;
	/* -----------------------------------------
	   "0.1" or argv[5]
	   "../testcases/n100.hardblocks" or argv[1]
	   "../testcases/n100.nets" or argv[2]
	   "../testcases/n100.pl" or argv[3]
	   ----------------------------------------- */
	dead_space_ratio = stof(argv[5]);					
	read_hardblocks.open(argv[1]);
	read_nets.open(argv[2]);		
	read_pl.open(argv[3]);	
	t2 = chrono::steady_clock::now();
	
	
	/* -----     read hardblocks     ----- */
	read_hardblocks >> S_temp >> S_temp >> block_num;
	read_hardblocks >> S_temp >> S_temp >> terminal_num;
	for ( i=0;i<block_num;i++ ) {
		
		Block b;
		read_hardblocks >> c >> c;
		read_hardblocks >> b.block_id;
		read_hardblocks >> S_temp >> c >> c >> fx >> c >> fy >> c;
		read_hardblocks >> S_temp >> S_temp >> c >> sx >> c >> sy >> c;
		read_hardblocks >> S_temp >> S_temp;
		
		b.width = sx - fx;
		b.height = sy - fy;
		
		blocks.push_back(b);
		id_blocks[b.block_id] = i;
		total_block_area += ( b.width*b.height );
		
	}
	
	
	/* -----     read terminals     ----- */
	i = 0;
	while ( terminal_num-- ) {
		
		Terminal t;
		read_pl >> c >> t.terminal_id >> t.x >> t.y;
		
		terminals.push_back(t);
		id_terminals[t.terminal_id] = i++;
		
	}
	

	/* -----     read nets     ----- */
	read_nets >> S_temp >> c >> net_num;
	read_nets >> S_temp >> c >> pin_num;
	for ( i=0;i<net_num;i++ ) {
		
		Net n;
		read_nets >> S_temp >> c >> k;
		for ( j=0;j<k;j++ ) {
			
			read_nets >> c;
			if ( c=='p' ) {
				
				read_nets >> temp;
				n.terminals.push_back(temp);
				
			}
			else {
				
				read_nets >> c >> temp;
				n.blocks.push_back(temp);
				
			}
			
		}
		
		nets.push_back(n);
		
	}
	t3 = chrono::steady_clock::now();
	construct_data_structure = chrono::duration_cast<chrono::microseconds>(t3-t2).count();
	
	/* -----     calculate floorplanning region     -----*/
	fp_region = ( total_block_area )*( 1+dead_space_ratio );
	w_fp = h_fp = sqrt(fp_region);
	
}


// write outputfile
void write(char *argv) {
	
	/* -------------------------------------
	   "../output/n100.floorplan" or argv[4]
	   ------------------------------------- */
	output_file.open(argv);
	output_file << "Wirelength " << final_WL << endl;
	output_file << "Blocks\n";
	for ( i=0;i<block_num;i++ ) {
		
		output_file << "sb" << blocks[i].block_id << " " << blocks[i].x << " " << blocks[i].y << " " << blocks[i].rotate << endl;
		
	}
	
}


// swap function
void swap(int s) {
	
	temp = blocks[s].width;
	blocks[s].width = blocks[s].height;
	blocks[s].height = temp;
	blocks[s].rotate = ( blocks[s].rotate + 1 )%2;
	
}


// construct initial tree
void initial_tree() {
	
//	cout << "initial tree" << endl;
	t1 = chrono::steady_clock::now();
	temp2 = 1;
	check_initial.assign(block_num,0);
	for ( i=0;i<net_num;i++ ) {
		
		for ( j=0;j<nets[i].blocks.size();j++ ) {
			
			k = id_blocks.find(nets[i].blocks[j])->second;
			if ( check_initial[k]==0 )  check_initial[k] = 1;
			else continue;
			if ( blocks[k].width > blocks[k].height ) swap(k);
			
			if ( temp2==1 ) {
				
				root = to_x = to_y = &blocks[k];
				t_x = blocks[k].width;
				temp2++;
				
			}
			else {
				
				if ( ( t_x+blocks[k].width )<w_fp ) {
					
					blocks[k].parent = to_x;
					to_x->l_child = &blocks[k];
					to_x = to_x->l_child;
					t_x += blocks[k].width;
					
				}
				else {
					
					blocks[k].parent = to_y;
					to_y->r_child = &blocks[k];
					to_y = to_y->r_child;
					to_x = to_y;
					t_x = blocks[k].width;
					
				}
				
			}
			
		}	
		
	}
	t3 = chrono::steady_clock::now();
	initial_floorplan = chrono::duration_cast<chrono::microseconds>(t3-t1).count();
	
}


// print tree
void print_tree(Block* r) {
	
	if (r!=NULL) {
		
		cout << "Block: " << r->block_id << " X: " << r->x << " Y: " << r->y << endl;
		cout << "Width: " << r->width << " Height : " << r->height << endl;
		cout << "Rotated : " << r->rotate << endl << endl;
		print_tree(r->l_child);
		print_tree(r->r_child);	
		
	}
	else cout << endl << "=======================" << endl;
	
}


// calculate each block's x , y and update contour
void calculate_xy() {
	
//	cout << "calculate xy contour" << endl;
	max_x = 0;
	contour.clear();
	contour.assign(w_fp*2,0);
	to_x = to_y = root;
	root->x = root->y = 0;
	t_x = root->width;
	for ( i=0;i<t_x;i++ ) contour[i] = root->height;
	
	for ( i=1;i<block_num;i++ ) {
		
		max_y = 0;
		
		if ( to_x->l_child!=NULL ) to_x = to_x->l_child;
		else {
			
			t_x = 0;
			to_y = to_y->r_child;
			to_x = to_y;
			
		}
		to_x->x = t_x;
		for ( j=t_x;j<t_x+to_x->width;j++ ) if ( contour[j]>max_y ) max_y = contour[j];
		for ( j=t_x;j<t_x+to_x->width;j++ ) contour[j] = max_y + to_x->height;
		t_x += to_x->width;
    if ( t_x>max_x ) max_x = t_x;
		to_x->y = max_y;
		
	}
	
	max_y = 0;
	for ( i=0;i<max_x;i++ ) if ( contour[i]>max_y ) max_y = contour[i];
	floorplanning_area = max_x*max_y;
	
}


// tree perturbation
void tree_perturbation() {
	
	perturb = rand()%10;
	
	/* -----     do rotation     ----- */
	if ( perturb<3 ) {
		
//		cout << "tree perturbation ( rotate )" << endl;
		rotate = rand()%block_num;
		swap(rotate);
		
	}
	/* -----     exchange blocks     ----- */
	else {
		
		a = rand()%block_num;
		b = rand()%block_num;
		while ( a==b ) b = rand()%block_num;
		
//		cout << "tree perturbation ( swap " << a << " and " << b << " )" << endl;
		
		a_p = blocks[a].parent;
		a_l = blocks[a].l_child;
		a_r = blocks[a].r_child;
		b_p = blocks[b].parent;
		b_l = blocks[b].l_child;
		b_r = blocks[b].r_child;
		
		// b is a's parent
		if ( a_p!=NULL ) {
			
			if ( a_p->block_id==blocks[b].block_id ) {
				
				if ( b_p==NULL ) root = &blocks[a];
				else if ( b_p->l_child->block_id==blocks[b].block_id ) b_p->l_child = &blocks[a];
				else b_p->r_child = &blocks[a];
	
				if ( a_l!=NULL ) a_l->parent = &blocks[b];
				if ( a_r!=NULL ) a_r->parent = &blocks[b];
				if ( b_l!=NULL ) {
					
					if ( b_l->block_id==blocks[a].block_id ) {
					
						if ( b_r!=NULL ) b_r->parent = &blocks[a];
						blocks[a].l_child = &blocks[b];
						blocks[a].r_child = b_r;
						
					}
					
				}
				if ( b_r!=NULL ) {
					
					if ( b_r->block_id==blocks[a].block_id ) {
					
						if ( b_l!=NULL ) b_l->parent = &blocks[a];
						blocks[a].r_child = &blocks[b];
						blocks[a].l_child = b_l;
						
					}
					
				}
				
				blocks[a].parent = b_p;
				blocks[b].parent = &blocks[a];
				blocks[b].l_child = a_l;
				blocks[b].r_child = a_r;
				return;
				
			}
			
		}
		
		// a is b's parent
		if ( b_p!=NULL ) {
			
			if ( b_p->block_id==blocks[a].block_id ) {
				
				if ( a_p==NULL ) root = &blocks[b];
				else if ( a_p->l_child->block_id==blocks[a].block_id ) a_p->l_child = &blocks[b];
				else a_p->r_child = &blocks[b];
	
				if ( b_l!=NULL ) b_l->parent = &blocks[a];
				if ( b_r!=NULL ) b_r->parent = &blocks[a];
				if ( a_l!=NULL ) {
					
					if ( a_l->block_id==blocks[b].block_id ) {
						
						if ( a_r!=NULL ) a_r->parent = &blocks[b];
						blocks[b].l_child = &blocks[a];
						blocks[b].r_child = a_r;
						
					}
					
				}
				if ( a_r!=NULL ) {
					
					if ( a_r->block_id==blocks[b].block_id ) {
						
						if ( a_l!=NULL ) a_l->parent = &blocks[b];
						blocks[b].r_child = &blocks[a];
						blocks[b].l_child = a_l;
						
					}
						
				}
				
				blocks[b].parent = a_p;
				blocks[a].parent = &blocks[b];
				blocks[a].l_child = b_l;
				blocks[a].r_child = b_r;
				return;
				
			}
			
		}
		
		// a and b has the same parent
		if ( a_p!=NULL and b_p!=NULL ) {
			
			if ( a_p->block_id==b_p->block_id ) {
				
				if ( a_p->l_child->block_id==blocks[a].block_id ) {
					
					a_p->l_child = &blocks[b];
					a_p->r_child = &blocks[a];
					
				}
				else {
					
					a_p->r_child = &blocks[b];
					a_p->l_child = &blocks[a];
					
				}
				if ( a_l!=NULL ) a_l->parent = &blocks[b];
				if ( a_r!=NULL ) a_r->parent = &blocks[b];
				if ( b_l!=NULL ) b_l->parent = &blocks[a];
				if ( b_r!=NULL ) b_r->parent = &blocks[a];
				blocks[a].l_child = b_l;
				blocks[a].r_child = b_r;
				blocks[b].l_child = a_l;
				blocks[b].r_child = a_r;
				return;
				
			}
			
		}
		
		// other situation
		if ( a_p==NULL ) root = &blocks[b];
		else if ( a_p->l_child->block_id==blocks[a].block_id ) a_p->l_child = &blocks[b];
		else a_p->r_child = &blocks[b];
		
		if ( b_p==NULL ) root = &blocks[a];
		else if ( b_p->l_child->block_id==blocks[b].block_id ) b_p->l_child = &blocks[a];
		else b_p->r_child = &blocks[a];
		
		if ( a_l!=NULL ) a_l->parent = &blocks[b];
		if ( a_r!=NULL ) a_r->parent = &blocks[b];
		if ( b_l!=NULL ) b_l->parent = &blocks[a];
		if ( b_r!=NULL ) b_r->parent = &blocks[a];
		
		blocks[a].parent = b_p;
		blocks[b].parent = a_p;
		blocks[a].l_child = b_l;
		blocks[b].l_child = a_l;
		blocks[a].r_child = b_r;
		blocks[b].r_child = a_r;
		
	}
	
}


// calculate HPWL
void HPWL() {
	
//	cout << "HPWL" << endl;
	wire_length = 0;
	
	for ( i=0;i<net_num;i++ ) {
		
		w_mx = w_my = w_fp;
		w_Mx = w_My = 0;
		nt = &nets[i];
		
		for ( j=0;j<nt->terminals.size();j++ ) {
			
			temp = id_terminals.find( nt->terminals[j] )->second;
			t_x = terminals[temp].x;
			t_y = terminals[temp].y;
			
			w_mx = min( t_x , w_mx );
			w_my = min( t_y , w_my );
			w_Mx = max( t_x , w_Mx );
			w_My = max( t_y , w_My );
			
		}
		
		for ( j=0;j<nt->blocks.size();j++ ) {
			
			temp = id_blocks.find(nt->blocks[j])->second;
			t_x = blocks[temp].x + blocks[temp].width/2;
			t_y = blocks[temp].y + blocks[temp].height/2;
			
			w_mx = min( t_x , w_mx );
			w_my = min( t_y , w_my );
			w_Mx = max( t_x , w_Mx );
			w_My = max( t_y , w_My );
			
		}
		
		wire_length += ( ( w_Mx-w_mx ) + ( w_My-w_my ) );
		
	}
	
}


// calculate cost
void cal_cost() { 
	
//	cout << "cost" << endl;
	cost = alpha*float(floorplanning_area)
		 + beta*float(wire_length);
	if ( max_x>w_fp ) cost += float(abs(max_x-w_fp))*500;
	if ( max_y>h_fp ) cost += float(abs(max_y-h_fp))*500;
		   
}


// simulated annealing
void SA() {
	
	t1 = chrono::steady_clock::now();
	SA_limit = inside_SA*block_num;
	
	while ( temperature>0.01 ) {
		
//		cout << endl << "temperature : " << temperature << endl;
		pre_x = final_x;
		pre_y = final_y;
			
		for ( SA_times=0;SA_times<SA_limit;SA_times++ ) {
			
//			cout << "SA_inner_loops : " << SA_times << endl;
			tree_perturbation();
			calculate_xy();
			HPWL();
			cal_cost();
			if ( cost<final_cost ) store_result();
			else restore_result();
			
		}
//		cout<<final_x<<" "<<final_y<<" "<<temperature<<endl;
		temperature *= cooling;
   
    	if ( pre_x==final_x and pre_y==final_y and float(rand()%100)<temperature/1000 ) {
    		
      		tree_perturbation();
			calculate_xy();
			HPWL();
			cal_cost();
      		store_result();
    
    	}
   
	}
	t3 = chrono::steady_clock::now();
	SA_part = chrono::duration_cast<chrono::microseconds>(t3-t1).count();
	
}


int main ( int argc , char *argv[] ) {
	
	/* -----     set random seed and read files     ----- */
	srand(seed);
	t1 = chrono::steady_clock::now();
	read_files(argv);
	t3 = chrono::steady_clock::now();
	auto io_time = chrono::duration_cast<chrono::microseconds>(t3-t1).count();
	
	
	/* -----     initial floorplan     ----- */
	initial_tree();
	calculate_xy();
	HPWL();
	cal_cost();
	store_result();
//	print_tree(root);
	cout << "====================================" << endl;
	cout << "Initial Floorplanning :" << endl << endl;
  	cout << "Area : " << fp_region << endl;
	cout << "X : " << w_fp << " , Y : " << h_fp << endl;
	cout << "Floorplanning Area : " << final_fpA << endl;
	cout << "Initial X : " << final_x << " , Y : " << final_y << endl;
	cout << "Total wire length: " << final_WL << endl;
	cout << "Alpha : " << alpha << " , Beta :  " << beta << " , Seed : " << seed << endl;
	cout << "====================================" << endl << endl;
	
	
	/* -----     simulated anealing     ----- */
	SA();
//	print_tree(root);
	
	
	/* -----     write files and print results     ----- */
	t1 = chrono::steady_clock::now();
	write(argv[4]);
	t3 = chrono::steady_clock::now();
	io_time += chrono::duration_cast<chrono::microseconds>(t3-t1).count();
	cout << endl << "==================================" << endl;
	cout << "Final Result :" << endl << endl;
	cout << "Floorplanning Area : " << final_fpA << endl;
	cout << "MAX_x : " << final_x << " , MAX_y : " << final_y << endl;
	cout << "Total wire length: " << final_WL << endl;
	cout << "I/O time : " << io_time << endl;
	cout << "Constructing data structure time : " << construct_data_structure << endl;
	cout << "Initial floorplanning time : " << initial_floorplan << endl;
	cout << "SA time : " << SA_part << endl;
	cout << "==================================" << endl << endl;
	
	return 0;
	
}
