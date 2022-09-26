#include <iostream>
#include <vector>
#include <fstream> 
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <algorithm>
#include <time.h>

using namespace std ;

struct ready{ // Ready Queue
	int index ;  
	int pid ;
	int priority ;
};

struct process{
    int pid ; 
    int cpu_burst_ori ; // 原本的 cpu burst time (計算turnaroundTime用) 
	int cpu_burst ; // 剩餘的 cpu burst time 
	int arrival ;
	int priority ;	
	int turnaroundTime ;
	double responseRatio ; // (cpu_burst + wait) / cpu_burst
	bool done ; // process 是否完成 
	bool arrivalOK ;
	string output ; // Gantt Chart
	
};

int gmethod = 0 ;  
int gtimeslice = 0 ;
int now = 0 ; // 目前時間 
vector<process> vec ; // input data 
vector<string> output ; // 每個method自己的 Gantt Chart
vector<ready> readyQ ; // RR 的 ready Queue 
vector<ready> PPRRQ ; // PPRR 的 ready Queue
vector<int> arrival ; // 所有process的 arrivaltime 
vector<vector<process>> alloutput ; // 所有method的輸出data 

static bool CompareByID( const process &a, const process &b ){
   return a.pid < b.pid ;
} 
    
static bool CompareByArrival( const process &a, const process &b ){
	if( a.arrival < b.arrival )
	    return true ;
	else if( a.arrival == b.arrival && a.pid < b.pid )
	    return true ;	
    else
        return false ;
} 

static bool CompareByCPUBurst( const process &a, const process &b ){
	if( a.cpu_burst < b.cpu_burst )
	    return true ;
	else if( a.cpu_burst == b.cpu_burst && a.arrival < b.arrival )
	    return true ;
	else if( a.cpu_burst == b.cpu_burst && a.arrival == b.arrival && a.pid < b.pid )
	    return true ;
    else
        return false ; 
}
 
static bool CompareByResponseRatio( const process &a, const process &b ){
	if( a.responseRatio > b.responseRatio )
	    return true ;
	else if( a.responseRatio == b.responseRatio && a.arrival < b.arrival )
	    return true ;
	else if( a.responseRatio == b.responseRatio && a.arrival == b.arrival && a.pid < b.pid)
	    return true ;	
    else
        return false ; 
}

static bool CompareByPriority( const ready &a, const ready &b ){ // stable
	return a.priority < b.priority ;
}

string intToString(int num){
    stringstream ss ;
    ss << num ;
    string str = ss.str() ;
    return str ;
    
}

string transform(int num){
	if( num >= 0 && num <= 9 )
	    return intToString(num) ;
    else if ( num == 10 ) return "A" ;
    else if ( num == 11 ) return "B" ;
    else if ( num == 12 ) return "C" ;
    else if ( num == 13 ) return "D" ;
    else if ( num == 14 ) return "E" ;
	else if ( num == 15 ) return "F" ;
	else if ( num == 16 ) return "G" ;
	else if ( num == 17 ) return "H" ;
    else if ( num == 18 ) return "I" ;
    else if ( num == 19 ) return "J" ;
    else if ( num == 20 ) return "K" ;
    else if ( num == 21 ) return "L" ;
    else if ( num == 22 ) return "M" ;
    else if ( num == 23 ) return "N" ;
    else if ( num == 24 ) return "O" ;
    else if ( num == 25 ) return "P" ;
    else if ( num == 26 ) return "Q" ;
    else if ( num == 27 ) return "R" ;
    else if ( num == 28 ) return "S" ;
    else if ( num == 29 ) return "T" ;
    else if ( num == 30 ) return "U" ;
    else if ( num == 31 ) return "V" ;
    else if ( num == 32 ) return "W" ;
    else if ( num == 33 ) return "X" ;
    else if ( num == 34 ) return "Y" ;
    else if ( num == 35 ) return "Z" ;
    else {
        cout << "transform ERROR!" << endl ;
        return "ERROR" ;
    }
	    
		
}

void readFile(string &filename){ 
	fstream file ;
	process p ;
	int front = -1 ;
    char ss[5] = ".txt" ;
    char ff[20] = "\0" ;
    string line = "\0" ;
    strcpy(ff,filename.c_str()) ;
    strcat(ff,ss) ;
	file.open (ff,ios::in) ;
	while(!file) { // file not found
		cout << "file not found!" << endl ;
		cout << "Input a file number: " ;
		cin >> ff ;
		filename = ff ;
        strcat(ff,ss) ;
		file.open(ff,ios::in) ;
	}
	file >> gmethod ;
	file >> gtimeslice ;
    for( int i = 0; i < 6; i++){
    	file >> line ;
	} 
	
	while( !file.eof() ){ 
		file >> p.pid ;
		if( p.pid == front ){ break ; }
		front = p.pid ;
		file >> p.cpu_burst ;
		p.cpu_burst_ori = p.cpu_burst ;
		file >> p.arrival ;
		file >> p.priority ;
		p.responseRatio = -1 ;
		p.done = false ;
		p.arrivalOK = false ;
		p.turnaroundTime = 0 ;
		vec.push_back(p) ;
	}
	file.close() ;
 
}

int findFirstArrival(int &j){ // FCFS 
	int i = 0 ;
	while( i < vec.size() ){
		if( vec.at(i).done == false ){
			j = i ;
			vec.at(i).done = true ;
			return vec.at(j).pid ;
		}
		i++ ;
	}
	cout << "FCFS ERROR!" ;
	return 0 ;

}

void FCFS(){
	int count = 0, pid = 0 ;
	string str = "\0" ;
	sort( vec.begin(), vec.end(), CompareByArrival ) ;
	while( count < vec.size() ){
		str = transform(findFirstArrival(pid)) ;
		if( now < vec.at(pid).arrival ){
			for(int k = 0; k < vec.at(pid).arrival-now; k++){
				output.push_back("-") ;
			}
			now = vec.at(pid).arrival ;
			vec.at(pid).arrivalOK = true ;
		}
		now = now + vec.at(pid).cpu_burst ;
		vec.at(pid).turnaroundTime = now - vec.at(pid).arrival ;
		for( int i = 0; i < vec.at(pid).cpu_burst; i++ ){
			output.push_back(str) ;
		}
		count++ ;
		pid = 0 ;
 	}
}

void addReadyQ(){ // RR 
	int i = 0 ;
	ready r ;
	while( i < vec.size() ){
		if( vec.at(i).arrival <= now && vec.at(i).arrivalOK == false ){
			r.pid = vec.at(i).pid ;
			r.index = i ;
			readyQ.push_back(r) ;
			vec.at(i).arrivalOK = true ;
		}
		i++ ;
	}
	 
}

void add(int i){ // RR 
	ready r ;
	r.pid = vec.at(i).pid ;
	r.index = i ;
	readyQ.push_back(r) ;	
}

void findReadyQ(int &index, int &pid){ // RR
	int i = 0 ;
	ready r ;

    if( readyQ.size() != 0 ){
    	index = readyQ.at(0).index ;
    	pid = readyQ.at(0).pid ;
    	readyQ.erase(readyQ.begin()) ;

	}
	else{ // readyQ 空 
		while( i < vec.size() ){
			if( vec.at(i).arrivalOK == false ){
				r.pid = vec.at(i).pid ;
				r.index = i ;
				readyQ.push_back(r) ;
				vec.at(i).arrivalOK = true ;
				findReadyQ(index,pid) ;
				break ;
			}
			i++ ;
		}
	}
}

bool allDone(){
	for(int i = 0; i < vec.size(); i++){
		if( vec.at(i).done == false ){
			return false ;
		}
	}
	return true ;
}

void RR(){
	int index = 0, pid = 0 ;
	string str = "\0" ;
    sort( vec.begin(), vec.end(), CompareByArrival ) ;
    addReadyQ() ;
    while( allDone() == false ){
    	
    	findReadyQ(index,pid) ;
    	str = transform(vec.at(index).pid) ;
		if( now < vec.at(index).arrival ){
			for(int k = 0; k < vec.at(index).arrival-now; k++){
				output.push_back("-") ;
			}
			now = vec.at(index).arrival ;
			vec.at(index).arrivalOK = true ;
		}
		
    	if( vec.at(index).cpu_burst <= gtimeslice ){
    		now = now + vec.at(index).cpu_burst ;
    		for( int i = 0; i < vec.at(index).cpu_burst; i++ ){
				output.push_back(str) ;
		    }
		    vec.at(index).turnaroundTime = now - vec.at(index).arrival ;
		    addReadyQ() ;
    		vec.at(index).cpu_burst = 0 ;
    		vec.at(index).done = true ;

		}
		else{
			vec.at(index).cpu_burst -= gtimeslice ;
			now = now + gtimeslice ;
			addReadyQ() ;
			add(index) ; //去後面排隊 
    		for( int i = 0; i < gtimeslice; i++ ){
				output.push_back(str) ;
		    }	
		}	
	}	
}

int findSRTF(){
	sort( vec.begin(), vec.end(), CompareByCPUBurst ) ;
	for(int i = 0; i < vec.size(); i++){
		if( vec.at(i).arrivalOK == false && vec.at(i).arrival <= now ){
			vec.at(i).arrivalOK = true ;
		}
	}
	//=================================================
	
	for(int i = 0; i < vec.size(); i++){
		if( vec.at(i).arrivalOK == true && vec.at(i).done == false ){
			return i ;
		}
	}
	// idle
	
	int min = 0 ;
	sort( vec.begin(), vec.end(), CompareByArrival ) ;
	for(int i = 0; i < vec.size(); i++){
		if( vec.at(i).done == false ){
			min = vec.at(i).arrival ;
			sort( vec.begin(), vec.end(), CompareByCPUBurst ) ;
			for( int i = 0; i < vec.size(); i++){
				if( vec.at(i).done == false && vec.at(i).arrival == min ){
					return i ;
				}
		    }
		}
	}
	
	cout << "error!<findSRTF>" << endl ;
	return 0 ;
		
}

int findNextArrival(){
	for( int i = 0; i < arrival.size(); i++ ){ 
		if( arrival.at(i) > now ){
			return arrival.at(i) ;
		}
	}
	return -1 ;
}

void SRTF(){
	sort( vec.begin(), vec.end(), CompareByArrival ) ;
	int index = 0, nextArrival = 0 ;
	int aa = vec.at(0).arrival ;
	string str = "\0" ;
    arrival.push_back(aa) ;
    for( int i = 0; i < vec.size(); i++ ){ 
    	if( vec.at(i).arrival != aa ){
    		arrival.push_back(vec.at(i).arrival) ;
		}
		aa = vec.at(i).arrival ;  
	}

    while( allDone() == false ){
    	index = findSRTF() ;
    	str = transform(vec.at(index).pid) ;
		if( now < vec.at(index).arrival ){
			for(int k = 0; k < vec.at(index).arrival-now; k++){
				output.push_back("-") ;
			}
			now = vec.at(index).arrival ;
			vec.at(index).arrivalOK = true ;
		}
		nextArrival = findNextArrival() ;
		if( now+vec.at(index).cpu_burst >= nextArrival && nextArrival != -1){

			vec.at(index).cpu_burst =  vec.at(index).cpu_burst - (nextArrival - now) ; 
    		for( int i = 0; i < nextArrival - now; i++ ){
				output.push_back(str) ;
		    }
			now = nextArrival ;

		}
		else{
    		for( int i = 0; i < vec.at(index).cpu_burst; i++ ){
				output.push_back(str) ;
		    }
			now+= vec.at(index).cpu_burst ;
			vec.at(index).cpu_burst = 0 ;
			
		}
		
		if( vec.at(index).cpu_burst == 0 ){
			vec.at(index).turnaroundTime = now - vec.at(index).arrival ;
			vec.at(index).done = true ;
		}		
    }
	
}

void addPPRRQ(){ // PPRR 
	int i = 0 ;
	ready r ;
	while( i < vec.size() ){
		if( vec.at(i).arrival <= now && vec.at(i).arrivalOK == false ){
			vec.at(i).arrivalOK = true ;
			r.index = i ;
			r.pid = vec.at(i).pid ;
			r.priority = vec.at(i).priority ;
			PPRRQ.push_back(r) ;
		}
		i++ ;
	}	 
}

void findPPRRQ(int &index){ // PPRR
	int i = 0 ;
	ready r ;
    if( PPRRQ.size() != 0 ){
    	sort( PPRRQ.begin(), PPRRQ.end(), CompareByPriority ) ;
    	index = PPRRQ.at(0).index ;
	}
	else{ // PPRRQ 空 
		sort( vec.begin(), vec.end(), CompareByArrival ) ;
		while( i < vec.size() ){
			if( vec.at(i).arrivalOK == false ){
				vec.at(i).arrivalOK = true ;
				r.index = i ;
				r.pid = vec.at(i).pid ;
				r.priority = vec.at(i).priority ;
				PPRRQ.push_back(r) ;
				findPPRRQ(index) ;
				break ;
			}
			i++ ;
		}
	}
}

bool doRRR( int priority ){
	for( int i = 1; i < PPRRQ.size(); i++ ){
		if( PPRRQ.at(i).priority == priority ){
			return true ;
		}
	}
	return false ;
}

void ADD( int index ){ // PPRR
	ready r ;
	r.index = index ;
	r.pid = vec.at(index).pid ;
	r.priority = vec.at(index).priority ;
	PPRRQ.push_back(r) ; // 去後面排隊
}

int highPriorityArrival( int priority, bool &equal ){ // 高優先權arrival 
	int arrival = 0 ;
	sort( vec.begin(), vec.end(), CompareByArrival ) ;
	for(int i = 0; i < vec.size(); i++){
		if( vec.at(i).arrivalOK == false ){
			if( vec.at(i).priority < priority ){
				arrival = vec.at(i).arrival ;
				equal = false ;
				return arrival ;				
			}
			else if( vec.at(i).priority == priority ){
				arrival = vec.at(i).arrival ;
				equal = true ;
				return arrival ;				
			}
		}
	}
	return -1 ;
}

void PPRR(){ 
	sort( vec.begin(), vec.end(), CompareByArrival ) ;
	int timeslice = gtimeslice ;
	int nextArrival = 0, tempPID = 0, remainTime = 0 ;
	string str = "\0" ;
	bool doRR = false, equal = false ;
    int index = 0, aa = 0 ;
	
    while( allDone() == false ){
    	addPPRRQ() ;
    	findPPRRQ(index) ; // PPRRQ.erase(PPRRQ.begin()) ;
    	str = transform(vec.at(index).pid) ;
		if( now < vec.at(index).arrival ){
			for(int k = 0; k < vec.at(index).arrival-now; k++){
				output.push_back("-") ;
			}
			now = vec.at(index).arrival ;
		}
		nextArrival = highPriorityArrival( vec.at(index).priority, equal ) ;
		doRR = doRRR(vec.at(index).priority) ;
		if( doRR == false ){
			if( now+vec.at(index).cpu_burst > nextArrival && nextArrival != -1 ){ // 有人來插隊 
				vec.at(index).cpu_burst =  vec.at(index).cpu_burst - (nextArrival - now) ; 
	    		for( int i = 0; i < nextArrival - now; i++ ){
					output.push_back(str) ;
			    }
			    
			    aa = ( nextArrival - now ) % gtimeslice ;
			    if( aa == 0 )
			    	remainTime = 0 ;
				else
					remainTime = gtimeslice - aa ;						
				
			    now = nextArrival ; 
			    addPPRRQ() ;
			    if( equal == false ){
			    	PPRRQ.erase(PPRRQ.begin()) ;
					ADD(index) ;
				}
			    tempPID = vec.at(index).pid ;
			    equal = false ;
				
			}
			else{
				tempPID = -1 ;
	    		for( int i = 0; i < vec.at(index).cpu_burst; i++ ){
					output.push_back(str) ;
			    }
				now+= vec.at(index).cpu_burst ;
				vec.at(index).cpu_burst = 0 ;
				vec.at(index).turnaroundTime = now - vec.at(index).arrival ;
				vec.at(index).done = true ; 
				PPRRQ.erase(PPRRQ.begin()) ;
			}

		}
		else{ // RR
 			if( tempPID == vec.at(index).pid && remainTime != -1 ){
 			 	gtimeslice = remainTime ;
			}
			if( nextArrival != -1 && ((vec.at(index).cpu_burst >= gtimeslice && now+gtimeslice > nextArrival) 
			    || (vec.at(index).cpu_burst < gtimeslice && now+vec.at(index).cpu_burst > nextArrival) ) ) {
				vec.at(index).cpu_burst =  vec.at(index).cpu_burst - (nextArrival - now) ; 
		    	for( int i = 0; i < nextArrival - now; i++ ){
					output.push_back(str) ;
				}
			    aa = ( nextArrival - now ) % gtimeslice ;
			    if( aa == 0 )
			    	remainTime = 0 ;
				else
					remainTime = gtimeslice - aa ;						
				
			    now = nextArrival ; 
			    addPPRRQ() ;
			    if( equal == false ){
			    	PPRRQ.erase(PPRRQ.begin()) ;
					ADD(index) ;
				}
			    tempPID = vec.at(index).pid ;
			    equal = false ;
			}
			else{
				tempPID = -1 ; 
			    if( vec.at(index).cpu_burst <= gtimeslice ){
			    	now = now + vec.at(index).cpu_burst ;
			    	for( int i = 0; i < vec.at(index).cpu_burst; i++ ){
						output.push_back(str) ;
					}
					vec.at(index).turnaroundTime = now - vec.at(index).arrival ;
			    	vec.at(index).cpu_burst = 0 ;
			   		vec.at(index).done = true ;	
					PPRRQ.erase(PPRRQ.begin()) ;	
				}
				else{
					vec.at(index).cpu_burst -= gtimeslice ;
					now = now + gtimeslice ;
					PPRRQ.erase(PPRRQ.begin()) ;
					addPPRRQ() ;
					ADD(index) ;
			    	for( int i = 0; i < gtimeslice; i++ ){
						output.push_back(str) ;
				    }						
				}					
			}
		}
		equal = false ;
		gtimeslice = timeslice ;
	}

}

void calResponseRatio(){
	
	double wait = 0 ;
	for(int i = 0; i < vec.size(); i++){
		if( vec.at(i).arrivalOK == false && vec.at(i).arrival <= now ){
			vec.at(i).arrivalOK = true ;
		}
	}
	int i = 0 ;
    while( i < vec.size() ){
    	if( vec.at(i).done == false && vec.at(i).arrivalOK == true ){
    		wait = now - vec.at(i).arrival ;
    		vec.at(i).responseRatio = ( vec.at(i).cpu_burst + wait) / vec.at(i).cpu_burst ;
		}
		i++ ;
	}	
}


int findHRRN(){
	sort( vec.begin(), vec.end(), CompareByResponseRatio ) ;
	for(int i = 0; i < vec.size(); i++){
		if( vec.at(i).arrivalOK == true && vec.at(i).done == false ){
			vec.at(i).done = true ;
			return i ;
		}
	}
	// idle
	sort( vec.begin(), vec.end(), CompareByArrival ) ;
	for(int i = 0; i < vec.size(); i++){
		if( vec.at(i).done == false ){
			vec.at(i).done = true ;
			return i ;
		}
	}
	
	cout << "error!<findHRRN>" << endl ;
	return 0 ;
		
}

void HRRN(){
	int count = 0 ;
	int index = 0 ;
	string str = "\0" ;
	while( count < vec.size() ){
		calResponseRatio() ;
		index = findHRRN() ;
		str = transform(vec.at(index).pid) ;
		if( now < vec.at(index).arrival ){
			for(int k = 0; k < vec.at(index).arrival-now; k++){
				output.push_back("-") ;
			}
			now = vec.at(index).arrival ;
			vec.at(index).arrivalOK = true ;
		}

		now = now + vec.at(index).cpu_burst ;
		vec.at(index).turnaroundTime = now - vec.at(index).arrival ;		
		for( int i = 0; i < vec.at(index).cpu_burst; i++ ){
			output.push_back(str) ;
		}
		count++ ;
 	}
		
}

void initialize(){
	now = 0 ;
	vec.clear() ;
	output.clear() ;
	readyQ.clear() ;
	arrival.clear() ;
}

void copy(){
	string str = "\0" ;
	for(int i = 0; i < output.size(); i++){
		str = str + output.at(i) ;
	}
	sort( vec.begin(), vec.end(), CompareByID ); 
	vec.at(0).output = str ;
	alloutput.push_back(vec) ;
	initialize() ;
}

void writeFile(string filename, string method) {
    fstream file ;
    string str = "out_" + filename + ".txt" ;
    file.open(str.c_str(),ios::out) ;
    if( method == "PPRR" )
    	file << "Priority RR" << endl ;
    else
	    file << method << endl ;
	    
    if( method == "RR" ) 
    	file << "==          " << method << "==" << endl ;
    else
	    file << "==        " << method << "==" << endl ;
	    
	for( int i = 0; i < output.size(); i++ ){
		file << output.at(i) ;
	}
	file << endl ;
	file << "===========================================================" << endl ;
	// waiting time============================
	file << endl << "Waiting Time" << endl ;
	file << "ID" << "\t" << method << endl ;
	file << "===========================================================" << endl ;
    sort( vec.begin(), vec.end(), CompareByID ); 
    for( int i = 0; i < vec.size(); i++ ){
    	file << vec.at(i).pid << "\t" << vec.at(i).turnaroundTime-vec.at(i).cpu_burst_ori << endl ;
	}
	file << "===========================================================" << endl ;
	
	// turnaround time =======================
	file << endl << "Turnaround Time" << endl ;
	file << "ID" << "\t" << method << endl ;
	file << "===========================================================" << endl ;
    for( int i = 0; i < vec.size(); i++ ){
    	file << vec.at(i).pid << "\t" << vec.at(i).turnaroundTime << endl ;
	}
	file << "===========================================================" << endl << endl ;
	
    file.close() ;
} 

void writeAllFile(string filename) { // ALL
    fstream file ;
    string str = "out_" + filename + ".txt" ;
    file.open(str.c_str(),ios::out) ;
    file << "All" << endl ;
    for( int i = 0; i < alloutput.size(); i++ ){
    	if( i == 0 )
    		file << "==        " << "FCFS" << "==" << endl ;
    	else if( i == 1 )
    		file << "==          " << "RR" << "==" << endl ;
    	else if( i == 2 )
    		file << "==        " << "SRTF" << "==" << endl ;
    	else if( i == 3 )
    		file << "==        " << "PPRR" << "==" << endl ;
    	else if( i == 4 )
    		file << "==        " << "HRRN" << "==" << endl ;
    	file << alloutput.at(i).at(0).output << endl ;
	}
	file << "===========================================================" << endl ;
	
	// waiting time============================
	file << endl << "Waiting Time" << endl ;
	file << "ID	FCFS	RR	SRTF	PPRR	HRRN" << endl ;
	file << "===========================================================" << endl ;
    
    for( int k = 0; k < alloutput.at(0).size(); k++ ){
    	file << alloutput.at(0).at(k).pid << "\t" ;
    	for( int i= 0; i < alloutput.size(); i++ ){
    		if( i == alloutput.size()-1 ){
    			file << alloutput.at(i).at(k).turnaroundTime-alloutput.at(i).at(k).cpu_burst_ori << "\n" ;
			} 
			else{
				file << alloutput.at(i).at(k).turnaroundTime-alloutput.at(i).at(k).cpu_burst_ori << "\t" ;
			}
             
	    }
//	    file << endl ;
    }
    file << "===========================================================" << endl ;

	// turnaround time =======================
	file << endl << "Turnaround Time" << endl ;
	file << "ID	FCFS	RR	SRTF	PPRR	HRRN" << endl ;
	file << "===========================================================" << endl ;
    for( int k = 0; k < alloutput.at(0).size(); k++ ){
    	file << alloutput.at(0).at(k).pid << "\t" ;
    	for( int i= 0; i < alloutput.size(); i++ ){
    		if( i == alloutput.size()-1 )
    			file << alloutput.at(i).at(k).turnaroundTime << endl ;
    		else
    			file << alloutput.at(i).at(k).turnaroundTime << "\t" ;
	    }
//	    file << endl ;
    }
    file << "===========================================================" << endl << endl;
	
    file.close() ;
} 

void ALL(string filename){
    vector <process> tt ; // 暫存原始資料
    tt.assign(vec.begin(), vec.end()) ;
	FCFS() ;
	copy() ;
	vec.assign(tt.begin(), tt.end()) ;
	RR() ;
	copy() ;
	vec.assign(tt.begin(), tt.end()) ;
	SRTF() ;
	copy() ;
	vec.assign(tt.begin(), tt.end()) ;
    PPRR() ;
	copy() ;
	vec.assign(tt.begin(), tt.end()) ;
    HRRN() ;
	copy() ;
	writeAllFile(filename) ;
}

int main(void) {
	string filename = "\0" ; 
	do {
	    cout << endl << "Input a file name [0: Quit]: " ;
	    cin >> filename ;
	    if( filename == "0" ){
	    	break ;
		}
		else{
            readFile(filename) ;   
            if( gmethod == 1 ){  	
            	FCFS() ;
            	writeFile(filename,"FCFS") ;    	
			} 
			else if( gmethod == 2 ){
				RR() ;
				writeFile(filename,"RR") ;
			}
			else if( gmethod == 3 ){
				SRTF() ;
				writeFile(filename,"SRTF") ;
			}
			else if( gmethod == 4 ){
				PPRR() ;
				writeFile(filename,"PPRR") ;
			}
			else if( gmethod == 5 ){
				HRRN() ;
				writeFile(filename,"HRRN") ;
			}
			else if( gmethod == 6 ){
				ALL(filename) ;
			}
			else{
				break ;
			}
			initialize() ;
			alloutput.clear() ;
		}

	} while (filename != "0"); 

	return 0;
} // end main
