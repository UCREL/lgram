<h2>This repository has moved to the UCREL Github</h2>
Please see <a href='https://github.com/UCREL'>github.com/UCREL</a> or <a href='http://ucrel.lancs.ac.uk/lgram/'>ucrel.lancs.ac.uk/lgram/</a> for more information.

Lgram is a cross–platform tool for calculating ngrams in a memory–efficient manner. The current crop of n-gram tools have non–constant memory usage such that ngrams cannot be computed for large input texts. Given the prevalence of large texts in computational and corpus linguistics, this deficit is problematic. Lgram has constant memory usage so it can compute ngrams on arbitrarily sized input texts. Lgram achieves constant memory usages by periodically syncing the computed ngrams to an sqlite database stored on disk.

Lgram was written by Edward J. L. Bell at Lancaster University and funded
by UCREL. The project was initiated by Dr Paul Rayson.