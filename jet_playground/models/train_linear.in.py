# first, a linear regression
from sklearn.linear_model import LinearRegression
from sklearn.model_selection import GridSearchCV
import numpy as np

''' grid search for a linear regressor '''
def train_linear_regressor( X, y, fit_intercept=[True], normalize=[False, True], copy_X=[True], verbose=1, n_jobs=1 ):
  param_grid = [ {'fit_intercept' : fit_intercept, 'normalize': normalize, 'copy_X' : copy_X  } ]
  linear_reg = LinearRegression()
  grid_search = GridSearchCV(linear_reg, param_grid, cv=5, scoring='neg_mean_squared_error', verbose=verbose, n_jobs=n_jobs)
  grid_search.fit( X, y )
  cvres = grid_search.cv_results_
  for mean_score, params in zip(cvres["mean_test_score"], cvres["params"]):
    print(np.sqrt(-mean_score), params)
  print(grid_search.best_estimator_)
  return grid_search.best_estimator_
