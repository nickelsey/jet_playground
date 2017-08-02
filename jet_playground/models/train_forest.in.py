from sklearn.ensemble import RandomForestRegressor
from sklearn.model_selection import GridSearchCV
import numpy as np

''' grid search for a random forest regressor '''
def train_forest( X, y, n_est=[ 3, 6, 10, 12, 15, 30 ], max_feat=[ 1, 3, 10, 20, 30 ], bootstrap=[ True, False ], random_state=[420], verbose=1, n_jobs=1 ):
  param_grid = [ {'bootstrap' : bootstrap, 'n_estimators': n_est, 'max_features' : max_feat, 'random_state' : random_state } ]
  forest_reg = RandomForestRegressor()
  grid_search = GridSearchCV(forest_reg, param_grid, cv=5, scoring='neg_mean_squared_error', verbose=verbose, n_jobs=n_jobs)
  grid_search.fit( X, y )
  cvres = grid_search.cv_results_
  for mean_score, params in zip(cvres["mean_test_score"], cvres["params"]):
      print(np.sqrt(-mean_score), params)
  print(grid_search.best_estimator_)
  return grid_search.best_estimator_

