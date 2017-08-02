from sklearn.tree import DecisionTreeRegressor
from sklearn.model_selection import GridSearchCV
import numpy as np

''' grid search for a decision tree regressor '''
def train_tree( X, y, max_depth=[ None, 2, 4, 8, 12 ], min_samples_split=[ 2, 4, 8, 12 ], max_feat=[ "auto", "log2", "sqrt" ], random_state=[420], verbose=1, n_jobs=1 ):
  param_grid = [ { 'max_features' : max_feat, 'min_samples_split' : min_samples_split, 'max_depth' : max_depth, 'random_state' : random_state } ]
  tree_reg = DecisionTreeRegressor()
  grid_search = GridSearchCV(tree_reg, param_grid, cv=5, scoring='neg_mean_squared_error', verbose=verbose, n_jobs=n_jobs)
  grid_search.fit( X, y )
  cvres = grid_search.cv_results_
  for mean_score, params in zip(cvres["mean_test_score"], cvres["params"]):
    print(np.sqrt(-mean_score), params)
  print(grid_search.best_estimator_)
  return grid_search.best_estimator_
