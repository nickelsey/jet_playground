from sklearn.neighbors import KNeighborsRegressor
from sklearn.model_selection import GridSearchCV
import numpy as np

''' grid search for a KNN regressor '''
def train_knn( X, y, n_neighbors=[1, 2, 3, 5, 10, 15, 20], weights=[ "uniform", "distance" ], random_state=[420], verbose=1, n_jobs=1 ):
  param_grid = [ { 'n_neighbors' : n_neighbors, 'weights' : weights, 'random_state' : random_state } ]
  knn_reg = KNeighborsRegressor()
  grid_search = GridSearchCV(knn_reg, param_grid, cv=5, scoring='neg_mean_squared_error', verbose=verbose, n_jobs=n_jobs)
  grid_search.fit( X, y )
  cvres = grid_search.cv_results_
  for mean_score, params in zip(cvres["mean_test_score"], cvres["params"]):
    print(np.sqrt(-mean_score), params)
  print(grid_search.best_estimator_)
  return grid_search.best_estimator_
