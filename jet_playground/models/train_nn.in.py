# train a neural network on jet correction data
# uses tensorflow to implement a simple feed forward
# neural network

import tools
import readTree as rt
import tensorflow as tf

def main( args ):
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


      ## ~~~~~~~ prepare data ~~~~~~~##
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







if __name__ == "__main__":
      parser = argparse.ArgumentParser(description='Train feed forward network with pythia/geant data')
      parser.add_argument('--poly', type=int, help=' create polynomials to order poly from all features')
      parser.add_argument('--maxjobs',type=int, help=' max number of parallel jobs to run')
      args = parser.parse_args()
      main( args )
