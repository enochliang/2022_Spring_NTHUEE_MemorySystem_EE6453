#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdlib>

using namespace std;


typedef struct{
    int serial_num;
    int thread_id;
    int bank;
    int row;
    bool rm;
}request;

typedef struct{
    int counter;// 0:idle, 1:the last busy cycle 
    bool row_hit;
    request req;
}bank_state;

class queue {
    private:
        int length;
        int size;
        request* req_queue;
    public:
        queue(int size_queue){
            size = size_queue;
            length = 0;
            req_queue = new request[size_queue];
            for(int i = 0; i < size ; i++){
                req_queue[i].rm=0;
            }
        }
        bool is_full(){
            if(length == size){return 1;}
            else{return 0;}
        }
        bool is_empty(){
            if(length == 0){return 1;}
            else{return 0;}
        }
        void push(request req_in){
            for(int i = length ; i>0 ; i--){
                req_queue[i] = req_queue[i-1];
            }
            req_queue[0] = req_in;
            length++;
        }
        request pop(request req_in){
            return req_queue[--length];
        }
        request get_req(int index){// (length > index >= 0)
            request tmp_req = req_queue[index];
            return tmp_req;
        }
        void rm_req(int index){
            for(int i = index ; i < (length - 1) ; i++){
                req_queue[i] = req_queue[i+1];
            }
            length--;
        }
        int FCFS_get_idx(int bank){
            
            for(int i = length - 1 ; i >= 0 ; i--){
                if(req_queue[i].bank == bank){return i;}
            }
            return -1;
        }
        int FR_FCFS_get_idx(int bank,int row){
            bool bank_match = 0;
            int idx = -1;
            for(int i = length - 1 ; i >= 0 ; i--){
                if((req_queue[i].bank == bank) && !bank_match){
                    idx = i;
                    bank_match = 1;
                }
                if((req_queue[i].bank == bank) && (req_queue[i].row == row)){return i;}
            }
            return idx;
        }
        int get_length(){
            return length;
        }
        void remove_element(){

            for(int i = size-1; i >= 0 ; i--){
                if(req_queue[i].rm){rm_req(i);req_queue[i].rm=0;}
            }
        }
        void set_to_rm(int index){
            req_queue[index].rm = 1;
        }
        
};

void print_queuing_info(request req){
    cout << "t" << setw(5) << req.serial_num;
    cout << "P" << setw(2) << req.thread_id;
    cout << "B" << setw(2) << req.bank;
    cout << "(" << setw(2) << req.row << ")";
}

void print_banks_info(bank_state*banks,int num_bank,int lat_rowhit,int lat_rowmiss){
    for(int i = 0; i < num_bank ; i++ ){// initializing state of banks
        if(banks[i].counter == 0){
            cout << "                   ";
        }else if(banks[i].counter == 1){
            cout << "    -------------- ";
        }else if(((banks[i].counter == lat_rowhit) && banks[i].row_hit) || ((banks[i].counter == lat_rowmiss) && !banks[i].row_hit) ){
            cout << "   ";
            print_queuing_info(banks[i].req);
        }else{
            cout << "   |              |";
        }
    }
    cout << "\n";
}


int main(){
    int num_process;
    int num_bank;
    int size_queue;
    int policy;
    int lat_rowhit;
    int lat_rowmiss;
    int marking_cap;
    int num_request;
    cin >> num_process >> num_bank >> size_queue >> policy >> lat_rowhit >> lat_rowmiss >> marking_cap >> num_request;
    cout << num_process << "\n";
    cout << num_bank << "\n";
    cout << size_queue << "\n";
    cout << policy << "\n";
    cout << lat_rowhit << "\n";
    cout << lat_rowmiss << "\n";
    cout << marking_cap << "\n";
    cout << num_request << "\n";

    int sim_cycles=0;
    int req_cnt=0;

    bool push_flag = 0;
    int idle_bank_cnt = 0;
    
    queue req_q(size_queue);

    
    request req_to_DRAM;


    bank_state banks[num_bank];// declare a set of banks.
    for(int i = 0; i < num_bank ; i++ ){// initializing state of banks
        banks[i].counter = 0;
    }

    int req_to_take_idx = -1;
    while(1){
        cout << setiosflags(ios::left);
        request req;
        
        req_q.remove_element();

        

        // update banks info
        for(int i = 0; i < num_bank ; i++ ){// Traversing all banks to find out idle bank(s).
            if(banks[i].counter != 0){
                banks[i].counter--;
            }
            if(banks[i].counter == 0 && !req_q.is_empty()){
                
                if(policy == 0)req_to_take_idx = req_q.FCFS_get_idx(i);
                else if(policy == 1)req_to_take_idx = req_q.FR_FCFS_get_idx(i,banks[i].req.row);
                
                if(req_to_take_idx >= 0){// there is a req can be take
                    req_to_DRAM = req_q.get_req(req_to_take_idx);
                    req_q.set_to_rm(req_to_take_idx);
                    /* print_queuing_info(req_to_DRAM);
                    cout<<"\n"; */
                    if(req_to_DRAM.row == banks[i].req.row){
                        banks[i].counter = lat_rowhit;
                        banks[i].row_hit = true;
                    }else{
                        banks[i].counter = lat_rowmiss;
                        banks[i].row_hit = false;
                    }
                    banks[i].req = req_to_DRAM;
                    //req_q.rm_req(req_to_take_idx);
                }
            }
        }

        
        
        // whether to fetch a req into queue
        if(!req_q.is_full() && (req_cnt != num_request)){
            cin >> req.serial_num >> req.thread_id >> req.bank >> req.row;
            req_cnt++;
            req.rm = 0;
            req_q.push(req);
            push_flag = 1;
        }else{
            push_flag = 0;
        }

        // whether the work finished.
        for(int i = 0; i < num_bank ; i++ ){
            if(banks[i].counter == 0){
                idle_bank_cnt++;
            }
        }
        if(req_q.is_empty() && (idle_bank_cnt == num_bank)){
            break;
        }
        else{
            idle_bank_cnt=0;

            cout << setiosflags(ios::left) << setw(7) << sim_cycles;
            if(push_flag){print_queuing_info(req);}
            else{cout<< "                ";}
            //cout<<req_q.get_length();
            print_banks_info(banks,num_bank,lat_rowhit,lat_rowmiss);
        }
        
        sim_cycles++;
    }

    return 0;
}
