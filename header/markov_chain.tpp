#include <stdexcept>

template <int  N>
MarkovChain<N>::MarkovChain(Eigen::Matrix<double,N,N> *M) {
    this->M=M;
    if(!this->is_schotastic()) {
        throw std::invalid_argument("Invalid argument: Provided Matrix is not a schotastic one.");
    }
    this->kosaraju_scc();
    this->set_nodes_periodicity();
}


template <int N>
MarkovChain<N>::~MarkovChain() {
    delete this->M;
}

template <int N>
bool MarkovChain<N>::is_schotastic() {
    //I use Schotastic-Cols not Schotastic-Rows
    for(int i = 0; i<N;i++) {
        double sigma = 0.0;
        for (int j = 0; j < N; j++) {
            sigma+= (*this->M)(j, i);
        }
        double err = std::abs(sigma-1);
        if(err > EPSILON) {
            return false; 
        }
    }

    return true;
}

template <int N>
void MarkovChain<N>::kosaraju_scc() {
    Eigen::Matrix<double,N,N> M_t = (*this->M).transpose();
    std::unordered_set<int> visited_first;
    std::stack<int> order_first;

    for(int i = 0; i < N;i++) {
        if(visited_first.find(i)==visited_first.end()) {
            this->depth_first_search((*this->M),order_first,visited_first,i);
        }
        if(order_first.size()==N) {
            break;
        }
    }
    this->scc=this->get_scc(M_t,order_first);
    this->set_node_types();
}

template <int N>
void MarkovChain<N>::depth_first_search(const Eigen::Matrix<double,N,N>& M,std::stack<int>& stack,std::unordered_set<int>&visited,int i) {
    std::vector<int> destinations = this->get_destinations(M,i);
    visited.emplace(i);
    for(int j = 0 ; j < destinations.size(); j++) {
        if(visited.find(destinations[j]) == visited.end()){
            this->depth_first_search(M,stack,visited,destinations[j]);
        }
    }   
    stack.push(i);
}

template<int N>
std::vector<int> MarkovChain<N>::get_destinations(const Eigen::Matrix<double,N,N>& M,int i) {
    std::vector<int> destinations; 
    for(int j = 0; j < N;j++) {
        if(i!=j&&M(j,i)>0) {
            destinations.push_back(j);
        }
    }
    return destinations;
}

template <int N>
std::vector<std::unordered_set<int>> MarkovChain<N>::get_scc(const Eigen::Matrix<double,N,N>&M,std::stack<int>&to_visit) {
    std::vector<std::unordered_set<int>> res; 
    std::unordered_set<int> visited;
    while(!to_visit.empty()) {
        int target = to_visit.top();
        std::unordered_set<int> component;
        if(visited.find(target) == visited.end()) {
            component.emplace(target);
            visited.emplace(target);
            this->nodes[target].id=target;
            this->nodes[target].component_id=res.size();
            std::stack<int> path;
            std::unordered_set<int> temp_visited; 
            this->depth_first_search(M,path,temp_visited,target);
            while(!path.empty()) {
                int second_target = path.top();
                if(visited.find(second_target) == visited.end()) {
                    visited.emplace(second_target);
                    component.emplace(second_target);
                    this->nodes[second_target].id=second_target;
                    this->nodes[second_target].component_id=res.size();
                }
                path.pop();
            }
            
        } 
        if(!component.empty()){
            res.push_back(component);
        }
        to_visit.pop();
    }
    return res;
}

template<int N>
void MarkovChain<N>::set_node_types() {
    for(int i = 0; i<this->scc.size();i++) {
        bool transient = false;
        for(auto it = this->scc[i].begin(); it!=this->scc[i].end();++it) {
            std::vector<int> destinations = this->get_destinations((*this->M),*it);

            for(int k = 0; k<destinations.size();k++) {
                if(this->nodes[destinations[k]].component_id != this->nodes[*it].component_id) {
                    transient = true;
                    break;
                }
            }
            if(transient) {
                //set the type to transient for the whole component
                for(auto it = this->scc[i].begin();it!=this->scc[i].end();++it) {
                    this->nodes[*it].type="transient";
                }
                break;
            }
        }
    }
}

template<int N>
void MarkovChain<N>::print_components() {
    for(int i = 0; i < this->scc.size();i++) {
        std::string type = "";
        std::cout << "{";
        for(auto it = this->scc[i].begin(); it!= this->scc[i].end();++it) {
            std::cout<<" "<<*it<<",";
            type = this->nodes[*it].type;
        }
        std::cout << " } : " << type <<std::endl;
    }
}
template <int N>
void MarkovChain<N>::print_nodes() {
    for(int i = 0; i < N; i++) {
        std::cout << i << " periodicity : "<< this->nodes[i].periodicity<<std::endl;
    }
}
template <int N>
Eigen::Vector<double,N> MarkovChain<N>::get_state_vector(long long n) {

    Eigen::Vector<double,N> p = Eigen::Vector<double,N>::Constant((1.0/(double)N));
    return this->get_state_vector(p);
     
}

template <int N>
Eigen::Vector<double,N> MarkovChain<N>::get_state_vector(const Eigen::Vector<double,N>& p,long long n) {
    Eigen::Vector<double,N> p_n = p;
    return m_pow(*this->M,n) * p_n;
}

template<int N>
void MarkovChain<N>::set_nodes_periodicity() {
    for(unsigned int i = 0; i < this->scc.size(); i++) {
        this->set_scc_periodicity(i);
    }
}

template <int N>
std::vector<unsigned int> MarkovChain<N>::get_neighbors(unsigned int i ) {
    std::vector<unsigned int> res; 
    for(unsigned int j = 0 ;  j < N;j++) {
        double p = (*this->M)(j,i);
        if(p > 0) {
            res.push(j);
        }
    }
    return res;
}
template <int N>
std::vector<unsigned int> MarkovChain<N>::get_neighbors_within_scc(unsigned int node_i,unsigned int component_i) {
    std::vector<unsigned int> res; 

    for(const auto& x : this->scc[component_i]) {

        double p = (*this->M)(x,node_i);
        if(p > 0) {
            res.push_back(x);
        }
    }
 
    return res;
}
template<int N>
std::vector<std::pair<unsigned int,unsigned int>> MarkovChain<N>::get_edges() { 
    std::vector<std::pair<unsigned int,unsigned int>> res;
    for(int i = 0; i < N;i++) {
        for(int j = 0;  j < N; j++) {
            double p =  (*this->M)(j,i);
            if(p>0) {
                res.push_back(std::make_pair<unsigned int,unsigned int>(i,j));
            }
        }
    }
    return res;
}

template<int N>
std::vector<std::pair<unsigned int,unsigned int>> MarkovChain<N>::get_edges_within_scc(unsigned int i) { 
    std::vector<std::pair<unsigned int,unsigned int>> res;
    for(const auto& u: this->scc[i]) {
        for(const auto& v: this->scc[i]) {
            double p = (*this->M)(u,v);
            if(p>0) {
                res.push_back(std::make_pair<unsigned int,unsigned int>(v,u));
            }
        }
    }
    return res;
}
template <int N>
void MarkovChain<N>::set_scc_periodicity(unsigned int i ) { 
    int target_node = *this->scc[i].begin();
    std::array<int,N> distances; //distance in a sense not weights but iterations
    distances.fill(-1);
    distances[target_node]=0;
    std::queue<unsigned int> queue;
    queue.push(target_node);
    while(!queue.empty()) {
        unsigned int u = queue.front();
        queue.pop();
        std::vector<unsigned int> neighbors = this->get_neighbors_within_scc(u,i);
        for(const auto& v : neighbors) {
            if(distances[v]==-1) {
                distances[v] = distances[u]+1;
                queue.push(v);
            }
        }
    }

     int periodicity = 0;
    auto edges = this->get_edges_within_scc(i);
    for(const auto& pair : edges) {
        if(distances[pair.first] == -1 || distances[pair.second] == -1) {
            continue;
        }
        int cycle = std::abs((distances[pair.first] + 1) - distances[pair.second]);
        if(cycle > 0 ) { 
            periodicity = std::gcd(periodicity,cycle);
        }
        if(periodicity==1) { //gcd(1,x) = 1
            break;
        }
    }
    for(const auto& x : this->scc[i]) {
        this->nodes[x].periodicity=periodicity;
    }
    
}
template <int N>
unsigned int MarkovChain<N>::get_periodicity(unsigned int i) {
    return this->nodes[i].periodicity;
}
template <int N>
bool MarkovChain<N>::is_aperiodic()  {
    if(this->is_aperiodic_calculated) {
        return this->is_aperiodic; 
    } else {
        for(int i = 0; i <N;i++) {
            if(this->nodes[i].periodicity!=1) {
                this->is_aperiodic_calculated=true;
                this->is_aperiodic_val=false;
                return this->is_aperiodic;
            }    
        }
        this->is_aperiodic_calculated=true;
        this->is_aperiodic_val=true;
        return this->is_aperiodic_val;
        
    }
}
template <int N>
bool MarkovChain<N>::is_irreducible() {
    return this->scc.size()==1;
}