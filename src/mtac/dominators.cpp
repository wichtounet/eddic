//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <vector>
#include <unordered_map>

#include "PerfsTimer.hpp"

#include "mtac/BasicBlock.hpp"
#include "mtac/Function.hpp"
#include "mtac/dominators.hpp"

using namespace eddic;

struct dominators {
    unsigned int* parent;
    unsigned int* semi;
    unsigned int* vertex;
    unsigned int* dom;
    unsigned int* size;
    unsigned int* child;
    unsigned int* label;
    unsigned int* ancestor;

    std::vector<unsigned int>* pred;
    std::vector<unsigned int>* succ;
    std::vector<unsigned int>* bucket;

    const std::size_t cn;
    mtac::function_p function;

    std::unordered_map<mtac::basic_block_p, unsigned int> numbers;
    std::unordered_map<unsigned int, mtac::basic_block_p> blocks;

    dominators(std::size_t cn, mtac::function_p function) : cn(cn), function(function) {
        parent = new unsigned int[cn+1];
        semi = new unsigned int[cn+1];
        vertex = new unsigned int[cn+1];
        dom = new unsigned int[cn+1];
        size = new unsigned int[cn+1];
        child = new unsigned int[cn+1];
        label = new unsigned int[cn+1];
        ancestor = new unsigned int[cn+1];

        succ = new std::vector<unsigned int>[cn+1];
        pred = new std::vector<unsigned int>[cn+1];
        bucket = new std::vector<unsigned int>[cn+1];
    }

    ~dominators(){
        delete[] parent;
        delete[] semi;
        delete[] vertex;
        delete[] dom;
        delete[] size;
        delete[] child;
        delete[] label;
        delete[] ancestor;

        delete[] bucket;
        delete[] pred;
        delete[] succ;
    }

    unsigned int n = 0;

    void dfs(unsigned int v){
        semi[v] = ++n;
        vertex[n] = label[v] = v;
        ancestor[v] = child[v] =0;
        size[v] = 1;

        for(auto w : succ[v]){
            if(semi[w] == 0){
                parent[w] = v;
                dfs(w);
            }

            pred[w].push_back(v);
        }
    }

    void compress(unsigned int v){
        if(ancestor[ancestor[v]] != 0){
            compress(ancestor[v]);

            if(semi[label[ancestor[v]]] < semi[label[v]]){
                label[v] = label[ancestor[v]];
            }

            ancestor[v] = ancestor[ancestor[v]];
        }
    }
    
    unsigned int eval(unsigned int v){
        if(ancestor[v] == 0){
            return label[v];
        } else {
            compress(v);

            if(semi[label[ancestor[v]]] > semi[label[v]]){
                return label[v];
            } else {
                return label[ancestor[v]];
            }
        }
    }

    void link(unsigned int v, unsigned int w){
        unsigned int s = w;
    
        while(semi[label[w]] < semi[label[child[s]]]){
            if(size[s] + size[child[child[s]]] >= 2 * size[child[s]]){
                ancestor[child[s]] = s;
                child[s] = child[child[s]];
            } else {
                size[child[s]] = size[s];
                s = ancestor[s] = child[s];
            }
        }

        label[s] = label[w];
        size[v] = size[v] + size[w];

        if(size[v] < 2 * size[w]){
            auto t = s;
            s = child[v];
            child[v] = t;
        }

        while(s != 0){
            ancestor[s] = v;
            s = child[s];
        }
    }

    void compute_dominators(){
        PerfsTimer timer("Dominators");

        /* Step 0. Translate basic blocks to numbers */

        unsigned int number = 0;
        for(auto& block : function){
            numbers[block] = ++number;
            blocks[number] = block;
        }
        
        number = 0;
        for(auto& block : function){
            ++number;
            for(auto& s : block->successors){
                succ[number].push_back(numbers[s]);
            }
        }

        /* Step 1. */

        for(unsigned int v = 1; v <= n; ++v){
            semi[v] = 0;
        }
    
        unsigned int n = 0;

        dfs(1);

        size[0] = label[0] = semi[0] = 0;

        for(unsigned int i = n; i >= 2; --i){
            auto w = vertex[i];
            
            /* Step 2 */

            for(auto v : pred[w]){
                auto u = eval(v);
                if(semi[u] < semi[w]){
                    semi[w] = semi[u];
                }
            }

            bucket[vertex[semi[w]]].push_back(w);

            link(parent[w], w);

            /* Step 3 */

            for(auto v : bucket[parent[w]]){
                //remove v from bucket[parent[w]]

                auto u = eval(v);

                if(semi[u] < semi[v]){
                    dom[v] = u;
                } else {
                    dom[v] = parent[w];
                }
            }
        }

        /* Step 4 */
    
        for(unsigned int i = 2; i <= n; ++i){
            unsigned w = vertex[i];
            if(dom[w] != vertex[semi[w]]){
                dom[w] = dom[dom[w]];
            }
        }

        dom[1] = 0;

        /* Step 5 */

        number = 0;
        for(auto& block : function){
            ++number;

            if(number == 1){
                block->dominator = nullptr;
            }

            //TODO Not sure if not necessary to use another array
            block->dominator = blocks[dom[number]]; 
        }
    }
};

void mtac::compute_dominators(std::shared_ptr<Function> function){
    auto n = function->bb_count();

    dominators dom(n, function);
    dom.compute_dominators();
}
