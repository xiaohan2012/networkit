import pytest
from networkit import Glist, Graph


@pytest.fixture
def g():
    g = Graph()
    for i in range(6):
        g.addNode()

    edges = [(0, 1), (0, 2), (1, 3), (2, 4), (1, 2), (3, 4), (2, 3), (4, 5)]
    for u, v in edges:
        g.addEdge(u, v)

    return g


@pytest.fixture
def glist(g):
    return Glist(g)


def test_compute_core(g, glist):
    assert glist.compute_core() == [2, 2, 2, 2, 2, 1]
    assert glist.compute_core() == glist.core


def test_insert_edge(g, glist):
    nodes = glist.insert_edge(0, 4)
    assert glist.core == [3]*5 + [1]
    assert set(nodes) == {0, 1, 2, 3, 4}
    nodes = glist.insert_edge(2, 5)
    assert glist.core == [3]*5 + [2]
    assert nodes == [5]


def test_remove_edge(g, glist):
    glist.insert_edge(0, 4)
    glist.insert_edge(2, 5)
    glist.remove_edge(0, 4)
    glist.remove_edge(2, 5)
    assert glist.core == [2]*5 + [1]


def test_fake_insert(g, glist):
    nodes = glist.fake_insert(0, 4)
    assert set(nodes) == {0, 1, 2, 3, 4}

    nodes = glist.fake_insert(2, 5)
    assert nodes == [5]

    # cores unchanged
    assert glist.compute_core() == [2, 2, 2, 2, 2, 1]
    assert glist.compute_core() == glist.core


def test_core_bfs(g, glist):
    glist.core_bfs()
    assert glist.sc_id == [0] * 5 + [1]

    glist.insert_edge(2, 5)
    glist.core_bfs()
    assert glist.sc_id == [0] * 6
