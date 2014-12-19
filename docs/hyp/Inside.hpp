boost::ptr_vector<Weight> costs;
insideAlgorithm(hyp, &costs);

for (unsigned i = 0, n = costs.size(); i < n; ++i)
  std::cout << i << " " << costs[i] << '\n';
