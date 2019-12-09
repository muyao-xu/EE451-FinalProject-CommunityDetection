import networkit as nk
import multiprocessing as mp
import jgraph as ig
import random

input_queue = mp.Queue()
output_queue = mp.Queue()



def SumUP(n, nprocs):
    for i in range(n):
        input_queue.put(i)
    for i in range(nprocs):
        input_queue.put('STOP')


def PLM(g, numiter, parallel=True):
    Elist = [g.es[i].tuple for i in range(len(g.es))]
    Nlist = range(g.vcount())

    loug = g.community_multilevel(return_levels=False)
    mod = g.modularity(loug)
    mod_res = mp.Value('d', mod)
    memship_res = mp.Array('i', loug.membership)

    if parallel:
        numprocs = mp.cpu_count() - 1
    else:
        numprocs = 1
    InQ_P = mp.Process(target=SumUP, args=(numiter - 1, numprocs))
    OutQ_P = mp.Process(target=OUTQ,args=(g, numprocs, mod_res, memship_res))
    ps = [mp.Process(target=comdet_worker, args=(Nlist, Elist))
          for k in range(numprocs)]

    InQ_P.start()
    OutQ_P.start()
    for p in ps:
        p.start()
    InQ_P.join()
    for p in ps:
        p.join()
    OutQ_P.join()

    print('Done.')
    return ig.VertexClustering(g, memship_res[:])


def comdet_worker(nlist, edlist):

    for i in iter(input_queue.get, "STOP"):
        rifrullo = range(len(nlist))
        random.shuffle(rifrullo)
        newedges = getNElist(edlist, rifrullo)
        getIgGraph = ig.Graph()
        getIgGraph.add_vertices(nlist)
        getIgGraph.add_edges(newedges)
        lougetIgGraph = getIgGraph.community_multilevel(return_levels=False)
        getIgGraphmod = getIgGraph.modularity(lougetIgGraph)
        membaux = lougetIgGraph.membership
        mshiplistaux = [membaux[rifrullo[i]] for i in range(len(membaux))]

        output_queue.put((getIgGraphmod, mshiplistaux))
    output_queue.put('STOP')


def getNElist(edlist, rifrullo):

    newedges = []
    for j in range(len(edlist)):
        newedges.append((rifrullo[edlist[j][0]], rifrullo[edlist[j][1]]))
    return newedges


def OUTQ(g, nprocs, mod_res, memship_res):
    for work in range(nprocs):
        for val in iter(output_queue.get, "STOP"):
            pass
            modaux = val[0]
            if modaux > mod_res.value:
                mod_res.value = modaux

                loug = ig.VertexClustering(g, val[1])

                memship_res[:] = loug.membership

filename = "Enter File Name Here"
G = nk.readGraph(filename, nk.Format.METIS)
maxiteration = 10000;
plm = PLM(G, maxiteration)
print(plm.membership)
