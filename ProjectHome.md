This paper presents a Connectivity-based and Anchor-free Three-dimensional Localization (CATL) scheme for large-scale sensor networks with concave regions. It distinguishes itself from previous work with a combination of three features:
(1) it works for networks in both 2D and 3D spaces, possibly
containing holes or concave regions;
(2) it is anchor-free, and uses only connectivity information to faithfully recover the original network topology, up to scaling and rotation;
(3) it does not depend on the knowledge of network boundaries, which suits it well to situations where boundaries are difficult to identify.
The key idea of CATL is to discover the notch nodes, where shortest
paths bend and hop-count-based distance starts to significantly
deviate from the true Euclidean distance. An iterative protocol is
developed that uses a notch-avoiding multilateration mechanism
to localize the network. Simulations show that CATL achieves
accurate localization results with a moderate per-node message
cost.