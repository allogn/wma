import sys
with open(sys.argv[1], "r") as f:
    with open("new_graph.ntw", 'w') as f2:
        info = f.readline().split()
        ecount = int(info[2])

        edge_set = set()
        edge_w = dict()
        for i in range(ecount):
            edgeinfo = f.readline().split()
            from_id = int(edgeinfo[0])
            to_id = int(edgeinfo[1])
            edge_weight = int(edgeinfo[2])
            if (from_id, to_id) not in edge_set and (to_id, from_id) not in edge_set:
                edge_set.add((from_id, to_id))
                edge_w[(from_id, to_id)] = edge_weight

        f2.write("%s %s %d %s\n" % (info[0], info[1], len(edge_set), info[3]))
        for e in edge_set:
            f2.write("%d %d %d\n" % (e[0], e[1], edge_w[e]))

        for l in f:
            f2.write(l)
