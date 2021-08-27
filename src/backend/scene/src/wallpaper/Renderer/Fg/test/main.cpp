#include "DependencyGraph.h"
#include "FrameGraph.h"
#include <iostream>
using namespace wallpaper::fg;

using namespace std;

void TestDependencyGraph() {
	DependencyGraph dgraph;
	auto n1 = dgraph.AddNode(std::make_unique<DependencyGraph::Node>());
	auto n2 = dgraph.AddNode(std::make_unique<DependencyGraph::Node>());
	auto n3 = dgraph.AddNode(std::make_unique<DependencyGraph::Node>());
	auto n4 = dgraph.AddNode(std::make_unique<DependencyGraph::Node>());
	auto n5 = dgraph.AddNode(std::make_unique<DependencyGraph::Node>());
	dgraph.Connect(n1, n2);
	dgraph.Connect(n2, n4);
	dgraph.Connect(n2, n3);
	dgraph.Connect(n3, n4);
	dgraph.Connect(n3, n5);
	dgraph.Connect(n4, n5);
	//dgraph.Connect(n3, n5);
	std::cout << "node:edge" << std::endl;
	std::cout << dgraph.NodeNum() << ":" << dgraph.EdgeNum() << std::endl;

	std::cout << "n3 input" << std::endl;
	for(auto& el:dgraph.GetNodeIn(n3)) {
		std::cout << el+1 << std::endl;
	}

	std::cout << "topological" << std::endl;
	auto top = dgraph.TopologicalOrder();
	for(auto& el:top) {
		std::cout << el+1 << std::endl;
	}
}


void TestFrameGraph() {
	/*
	FrameGraph fgraph;
	struct PassData {
		FrameGraphResource input;
		FrameGraphMutableResource output;
	};
	auto& pass = fgraph.AddPass<PassData>("test",
		[&](FrameGraphBuilder& builder, PassData& data) {
			data.input = builder.Read(data.input);
			data.output = builder.Write(data.output);
		},
		[=]() {}
	);
	struct PassData2 {
		FrameGraphResource input;
		FrameGraphMutableResource output;
	};
	auto& pass2 = fgraph.AddPass<PassData2>("test2",
		[&](FrameGraphBuilder& builder, PassData2& data) {
			data.input = builder.Read(pass->output);
			builder.Read(pass->input);
			data.output = builder.Write(data.output);
		},
		[=]() {}
	);
	std::cout << "topological" << std::endl;
	auto top = fgraph.Graph().TopologicalOrder();
	for(auto& el:top) {
		std::cout << el+1 << std::endl;
	}*/
}

int main(void) {
	TestDependencyGraph();
	cout << endl;
	TestFrameGraph();
	return 0;
}	