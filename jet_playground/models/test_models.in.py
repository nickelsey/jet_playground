import numpy as np
import ROOT
import tensorflow as tf
import pandas
import sys
import os
import argparse
import matplotlib.pyplot as plt
from pandas.plotting import scatter_matrix
from sklearn.model_selection import StratifiedShuffleSplit
from sklearn.preprocessing import StandardScaler
from sklearn.metrics import mean_squared_error
from sklearn import linear_model
from sklearn.preprocessing import PolynomialFeatures
from sklearn.neighbors import KNeighborsRegressor

# first, a linear regression
from sklearn.linear_model import LinearRegression
from sklearn.model_selection import cross_val_score
# now lets try a random forest with a grid search for hyperparameters
from sklearn.model_selection import GridSearchCV
from sklearn.ensemble import RandomForestRegressor

# try a decision tree regressor
from sklearn.tree import DecisionTreeRegressor

# testing my new stuff
import tools
import train_linear as linear
import train_forest as forest
import train_tree as tree
import train_kn as knn
import readTree as rt
import transforms

def main( args ):
  
      ## parse command line arguments
      ## use command line options to define which models
      ## will be trained
      use_linear = args.linear
      use_nn = args.NN
      use_forest = args.forest
      use_tree = args.tree
      use_knn = args.knn
      
      ## use poly to define the max order of polynomials to be used
      ## in training
      if args.poly is not None:
          poly_order = args.poly
      else:
          poly_order = 1
      
      ## pull the maximum number of parallel jobs
      if args.maxjobs is not None:
          max_jobs = args.maxjobs
      else:
          max_jobs = 2
  
      ## 'strings' should hold the paths to input TTree ROOT files
      ## if empty, will use default
      files = args.strings
      if files == "" or files is None or files == []:
        files = ["${CMAKE_BINARY_DIR}/training/antikt_R_0.4_inc_0_charged_0.root"]



      ## check if the files all have the same jetfinding options, if not, discard
      files = rt.validateFiles( files )

      ## reading trees into pandas dataframes
      jet_data = rt.load_root_tree( files, tree="training" )

      ## showing the statistics on the raw data
      jet_data.info()
      jet_data.describe()

      ## test the split_by_bin function
      pt_column_name = "pt"
      n_pt_bins = 24
      pt_bin_min = 2
      pt_bin_max = 50
      rndm_state = 420
      pt_bins = tools.split_by_bin( jet_data, pt_column_name, n_pt_bins, pt_bin_min, pt_bin_max )
      pt_scaled, excess_data = tools.recombine_bins_equal_entries( pt_bins, random_state=rndm_state )
      
      ## split into train & test data, to do so we need an index
      pt_scaled['index'] = [ i for i in range(1,pt_scaled.shape[0]+1) ]
      
      ## split by hashing the index, which keeps the training set equavalent
      ## between runs, as long as the dataframe has only been appended to
      ## and not shuffled
      test_ratio = 0.2
      train_data, test_data = tools.split_train_test_by_id(pt_scaled, test_ratio, "index" )
      
      ## split out the data we want to build models with, and labels
      input_columns = [ "pt", "eta", "phi", "ncharge", "charge_frac", "area" ]
      output_columns = [ "reco_pt" ]
      X_train, y_train = train_data[ input_columns ], train_data[ output_columns ]
      X_test,  y_test  = test_data[ input_columns ], test_data[ output_columns ]
      
      ## ~~~~~~~~~~~~        SCALING        ~~~~~~~~~~~##
      ## nothing is terribly beyond values of 0-10, for now don't scale
      pipeline = transforms.build_transform(poly_order)
      polynomial_transform = PolynomialFeatures(degree=poly_order)
      X_test_prepared = polynomial_transform.fit_transform( X_test )
      X_train_prepared = polynomial_transform.fit_transform( X_train )
      


      ## build our output dataframe
      df_out = pandas.DataFrame()
      df_out['pythia_pt'] = y_test['reco_pt']
      df_out['geant_pt'] = X_test['pt']
      ## ~~~~~~~~~~~~        TESTING        ~~~~~~~~~~~##
      # linear models
      if use_linear:
          print( "Training linear models")
          best_linear_model = linear.train_linear_regressor( X_train_prepared, y_train, fit_intercept=[True], n_jobs=max_jobs )
          linear_predictions = np.array(best_linear_model.predict( X_test_prepared )).squeeze()
          tools.compare_model_to_geant( y_test['reco_pt'], geant=X_test['pt'], model=linear_predictions, model_name="Linear" )
          df_out['linear_pt'] = linear_predictions
      
      # ensemble methods
      if use_forest:
          print("training random forest models")
          if X_train_prepared.shape[1] <= 10:
            n_features = X_train_prepared.shape[1]
          else:
            n_features = [ i for i in range(1, X_train_prepared.shape[1], int(X_train_prepared.shape[1]/10)) if i < X_train_prepared.shape[1]  ]
          best_forest = forest.train_forest( X_train_prepared, y_train['reco_pt'], max_feat=n_features, n_est=[ 3, 6, 10, 15, 20, 30 ], n_jobs=max_jobs )
          forest_predictions = np.array(best_forest.predict(X_test_prepared)).squeeze()
          tools.compare_model_to_geant( y_test['reco_pt'], geant=X_test['pt'], model=forest_predictions, model_name="Forest" )
          df_out['forest_pt'] = forest_predictions

      # decision tree models
      if use_tree:
          print( "Training decision tree models")
          best_tree_model = tree.train_tree( X_train_prepared, y_train, n_jobs=3 )
          tree_predictions = np.array(best_tree_model.predict( X_test_prepared )).squeeze()
          tools.compare_model_to_geant( y_test['reco_pt'], geant=X_test['pt'], model=tree_predictions, model_name="Decision Tree", n_jobs=max_jobs )
          df_out['tree_pt'] = tree_predictions

      # KNN models
      if use_knn:
          print( "Training KNN models")
          best_knn_model = knn.train_knn( X_train_prepared, y_train, n_jobs=3 )
          knn_predictions = np.array(best_knn_model.predict( X_test_prepared )).squeeze()
          tools.compare_model_to_geant( y_test['reco_pt'], geant=X_test['pt'], model=knn_predictions, model_name="Decision Tree", n_jobs=max_jobs )
          df_out['knn_pt'] = knn_predictions

      ## write file out
      rt.write_to_file( df_out, tree_name="test" )


if __name__ == "__main__":
      parser = argparse.ArgumentParser(description='Train with pythia/geant data')
      parser.add_argument('strings', metavar='S', nargs='*', help=' paths to root files ')
      parser.add_argument('--linear', type=bool, help=' train linear model')
      parser.add_argument('--forest', type=bool, help=' train random forest')
      parser.add_argument('--tree', type=bool, help=' train decision tree' )
      parser.add_argument('--knn', type=bool, help=' train KNN regressor' )
      parser.add_argument('--NN', type=bool, help=' train neural network')
      parser.add_argument('--poly', type=int, help=' create polynomials to order poly from all features')
      parser.add_argument('--maxjobs',type=int, help=' max number of parallel jobs to run')
      args = parser.parse_args()
      main( args )
