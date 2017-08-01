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
import readTree as rt
import transforms

def main( args ):
  
      ## parse command line arguments
      ## use command line options to define which models
      ## will be trained
      use_linear = args.linear
      use_nn = args.NN
      use_forest = args.forest
      
      ## use poly to define the max order of polynomials to be used
      ## in training
      if args.poly is not None:
          poly_order = args.poly
      else:
          poly_order = 1
      
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
      pt_scaled.info()
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
      X_train_prepared = polynomial_transform.fit_transform( X_train )
      X_test_prepared = polynomial_transform.fit_transform( X_test )


      ## build our output dataframe
      df_out = pandas.DataFrame()
      df_out['pythia_pt'] = y_test['reco_pt']
      df_out['geant_pt'] = X_test['pt']
      ## ~~~~~~~~~~~~        TESTING        ~~~~~~~~~~~##
      # linear models
      if use_linear:
          print( "Training linear models")
          best_linear_model = linear.train_linear_regressor( X_train_prepared, y_train, fit_intercept=[True] )
          linear_predictions = np.array(best_linear_model.predict( X_test_prepared )).squeeze()
          tools.compare_model_to_geant( y_test['reco_pt'], geant=X_test['pt'], model=linear_predictions, model_name="Linear" )
          df_out['linear_pt'] = linear_predictions
      
      # ensemble methods
      if use_forest:
          print("training random forest models")
          best_forest = forest.train_forest( X_train_prepared, y_train['reco_pt'], max_feat=[ 1, 2, 3, 4, 5, 6 ], n_est=[ 3, 6, 10, 12, 15 ], n_jobs=3 )
          forest_predictions = np.array(best_forest.predict(X_test_prepared)).squeeze()
          tools.compare_model_to_geant( y_test['reco_pt'], geant=X_test['pt'], model=forest_predictions, model_name="Forest" )
          df_out['forest_pt'] = forest_predictions


      ## write file out
      rt.write_to_file( df_out, tree_name="test" )
'''
      ## build an index
      train_pt_norm['index'] = range(1, len(train_pt_norm) + 1)
    
      corr = train_pt_norm.corr()
      print ( corr['reco_pt'].sort_values(ascending=False) )

      ## do a split for training & test data
      train_set, test_set = rt.split_train_test_by_id( train_pt_norm, 0.2, "index" )

      train_set = train_set.drop( "index", axis=1 )
      test_set = test_set.drop( "index", axis=1 )

      ## split out labels
      print("number of training examples: ", train_set.size )
      X_train = train_set[["pt", "npart", "charge_frac", "area", "phi", "eta"]] # "charge_frac", "area
      y_train = train_set["reco_pt"]
      X_test =  test_set[["pt", "npart", "charge_frac", "area", "phi", "eta"]]
      y_test =  test_set["reco_pt"]


      scaler = StandardScaler()
      poly = PolynomialFeatures(degree=3)
      ## scale the input
      X_train_scaled = poly.fit_transform(scaler.fit_transform( X_train ))
      X_test_scaled = poly.fit_transform(scaler.fit_transform( X_test ))
      #X_train_scaled = X_train
      # X_test_scaled = X_test


      
      ## raw RMSE from no correction
      mse_raw = mean_squared_error( y_test, X_test['pt'] )
      train_mse_raw = mean_squared_error( y_train, X_train['pt'] )
      rel_err_raw = rt.mean_absolute_error( y_test, X_test['pt'] )
      train_rel_err_raw = rt.mean_absolute_error( y_train, X_train['pt'] )
      print("RAW  test rmse: ", np.sqrt(mse_raw) )
      print("RAW  train rmse: ", np.sqrt(train_mse_raw) )
      print("RAW  test rel error: ", rel_err_raw )
      print("RAW  train rel error: ", train_rel_err_raw )

      plt.hexbin(X_test['pt'], y_test,gridsize=50, cmap='inferno', bins='log' )
      plt.savefig("before_correction.pdf")

      ## train a linear model
      lin_reg = LinearRegression()
      lin_reg.fit(X_train_scaled, y_train)
      lin_reg_pred = lin_reg.predict( X_test_scaled )
      lin_rmse = np.sqrt(mean_squared_error(lin_reg_pred, y_test) )
      lin_rel_err = rt.mean_absolute_error( lin_reg_pred, y_test )
      lin_scores = cross_val_score(lin_reg, X_train_scaled, y_train,
                                   scoring="neg_mean_squared_error", cv=10 )
      print("linear model rmse: ", lin_rmse )
      print("linear model rel err: ", lin_rel_err )

      tree_reg = DecisionTreeRegressor()
      tree_reg.fit( X_train_scaled, y_train )
      tree_pred = tree_reg.predict( X_test_scaled )
      tree_mse = mean_squared_error( tree_pred, y_test )
      tree_rmse = np.sqrt( tree_mse )
      tree_rel_err = rt.mean_absolute_error( tree_pred, y_test )
      print("decision tree model error: ", tree_rmse )
      print("decision tree rel err: ", tree_rel_err )

      bayes_reg = linear_model.BayesianRidge()
      bayes_reg.fit( X_train_scaled, y_train )
      bayes_pred = bayes_reg.predict( X_test_scaled )
      bayes_mse = mean_squared_error( bayes_pred, y_test )
      bayes_rmse = np.sqrt(bayes_mse)
      bayes_rel_err = rt.mean_absolute_error( bayes_pred, y_test )
      print("bayes regularization rmse: ", bayes_rmse )
      print("bayes regulariztion rel err: ", bayes_rel_err )
      
      # kneighbors
      kn_reg = KNeighborsRegressor()
      kn_reg.fit( X_train_scaled, y_train )
      kn_pred = kn_reg.predict( X_test_scaled )
      kn_mse = mean_squared_error( kn_pred, y_test )
      kn_rmse = np.sqrt( kn_mse )
      kn_rel_err = rt.mean_absolute_error( kn_pred, y_test )
      print("KNearestNeighbors rmse: ", kn_rmse )
      print("KNearestNeighbors rel err: ", kn_rel_err )

      final_model = rt.train_forest( X_train_scaled, y_train )
      final_predictions = final_model.predict(X_test_scaled)
      final_mse = mean_squared_error(final_predictions, y_test)
      final_rmse = np.sqrt(final_mse)
      final_rel_error = rt.mean_absolute_error( final_predictions, y_test )

      print("test set RMSE: ", final_rmse)
      print("test set rel err: ", final_rel_error )

      plt.hexbin(final_predictions, y_test,gridsize=50, cmap='inferno', bins='log' )
      plt.savefig("after_correction.pdf")

      fig, axs = plt.subplots(ncols=2, sharey=True, figsize=(14, 8))
      fig.subplots_adjust(hspace=0.5, left=0.07, right=0.93)
      ax = axs[0]
      hb = ax.hexbin(X_test['pt'], y_test,gridsize=50, cmap='inferno',bins='log')
      ax.axis([0, 60, 0, 60])
      ax.set_title("Raw Jet pt")
      cb = fig.colorbar(hb, ax=ax)
      cb.set_label('log10(N)')

      ax = axs[1]
      hb = ax.hexbin(final_predictions, y_test, gridsize=50, bins='log', cmap='inferno')
      ax.axis([0, 60, 0, 60])
      ax.set_title("Random Forest Prediction")
      cb = fig.colorbar(hb, ax=ax)
      cb.set_label('log10(N)')
      plt.savefig("total.pdf")

'''


if __name__ == "__main__":
      parser = argparse.ArgumentParser(description='Train with pythia/geant data')
      parser.add_argument('strings', metavar='S', nargs='*', help=' paths to root files ')
      parser.add_argument('--linear', type=bool, help=' train linear model')
      parser.add_argument('--forest', type=bool, help=' train random forest')
      parser.add_argument('--NN', type=bool, help=' train neural network')
      parser.add_argument('--poly', type=int, help=' create polynomials to order poly from all features')
      args = parser.parse_args()
      main( args )
