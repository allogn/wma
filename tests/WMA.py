import networkx as nx
import unittest
import subprocess
import sys
import os
sys.path.append(os.path.join(os.environ['PHD_ROOT'],'wma','scripts'))
data_path = os.path.join(os.environ['ALLDATA_PATH'],'wma')
import helpers
import pandas as pd
import logging

root = logging.getLogger()
root.setLevel(logging.DEBUG)

class TestWMA(unittest.TestCase):
    def test_WMA_vs_Gurobi(self):
        G = nx.grid_2d_graph(3,3)
        G = nx.convert_node_labels_to_integers(G)
        G.add_edges_from([(9,10),(10,11),(0,12)])
        self.assertSetEqual(set(G.neighbors(0)), set([1,3,12]))
        weights = {
            (0,3): 6,
            (0,1): 1,
            (1,4): 2,
            (1,2): 12,
            (2,5): 13,
            (3,6): 30,
            (3,4): 7,
            (4,7): 20,
            (4,5): 3,
            (5,8): 4,
            (6,7): 11,
            (7,8): 5,
            (9,10): 30,
            (10,11): 40,
            (0,12): 0
        }
        nx.set_edge_attributes(G, weights, 'weight')
        nx.set_node_attributes(G, 0, 'lat')
        nx.set_node_attributes(G, 0, 'lon')
        sources = [0, 10, 12]
        targets = [7, 5, 9]
        number_of_facilities = len(targets)

        graph_file = os.path.join(data_path, "tmp.ntw")
        facility_file = os.path.join(data_path, "tmp_fac.ntw")
        outfile_wma = os.path.join(data_path, "tmp_wma.txt")
        outfile_gurobi = os.path.join(data_path, "tmp_gurobi.txt")

        helpers.export_graph(G, graph_file, 0, sources)
        pot_facilities = pd.DataFrame(list(zip(targets, [1]*len(targets))))
        pot_facilities.to_csv(facility_file, header=False, index=False, sep=' ')
        res_wma = helpers.run_and_wait_wma(graph_file, 100, number_of_facilities, outfile_wma, facility_file)
        logging.info("WMA result: %s" % res_wma)
        self.assertTrue(res_wma)
        res_gurobi = helpers.run_and_wait_gurobi(graph_file, 100, number_of_facilities, outfile_gurobi, facility_file)
        logging.info("Gurobi result: %s" % res_gurobi)
        self.assertTrue("Gurobi result: %s" % res_gurobi)

        wma_results = helpers.load_result(outfile_wma)
        gurobi_results = helpers.load_result(outfile_gurobi)

        self.assertEqual(wma_results['objective'], gurobi_results['objective'])
        self.assertEqual(wma_results['objective'], 51)
