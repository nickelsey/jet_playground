import numpy as np
import hashlib
import pandas

''' Used internally to check the hash of an entry's index 
    to determine if it will be in the test set or training set. '''
def test_set_check( identifier, test_ratio, hash ):
  return int(hash( np.int64(identifier)).hexdigest()[-1]) < 256 * test_ratio

''' splits a data set into two sets with relative sizes of test_ration*n_entries
    and (1-test_ratio)*n_entries, where each item will always appear in the same
    set as long as the index does not change. '''
def split_train_test_by_id( data, test_ratio, id_column, hash=hashlib.md5 ):
  ids = data[id_column]
  in_test_set = ids.apply( lambda id_: test_set_check( id_, test_ratio, hash ) )
  return data.loc[~in_test_set], data.loc[in_test_set]

''' prints cross validation scores, their means and the standard deviation. '''
def display_cv_scores(scores):
  print("Scores: ", scores)
  print("Mean: ", scores.mean())
  print("STD Dev: ", scores.std() )

''' returns the mean relative error of two sets of numbers with respect to
  y_true. Does not check for y_true == 0, so be careful and only use when
  appropriate. '''
def mean_absolute_error(y_true, y_pred):
  return np.mean(np.abs((y_true - y_pred) / y_true))

''' returns the average difference between y_true & y_pred '''
def average_difference(y_true, y_pred):
  return np.mean( np.subtract(y_true, y_pred) )

''' compares a model's predicted jet pt to the truth and the raw jet pt to see
    how much the model improves over no correction '''
def compare_model_to_geant( pythia, geant=None, model=None, model_name=None ):

    print( "Comparison of geant and model predictions to Pythia" )
    if model_name is not None:
        print("Model: ", model_name )
    print( "Pythia - mean pT in sample: ", np.mean(pythia) )
    print( "Pythia - RMS in sample: ", np.std(pythia) )
    if geant is not None:
        print( "Geant - mean pT in sample: ", np.mean(geant) )
        print( "Geant - RMS in sample: ", np.std(geant) )
        print( "Geant - mean pT difference in sample: ", np.mean( np.subtract(geant, pythia) ) )
        print( "Geant - RMS of pT difference in sample: ", np.std( np.subtract(geant, pythia) ) )
        print( "Geant - mean relative error in sample: ", mean_absolute_error( geant, pythia ) )
    if model is not None:
        print( model_name, "- mean pT in sample: ", np.mean(model) )
        print( model_name, "- RMS in sample: ", np.std(model) )
        print( model_name, "- mean pT difference in sample: ", np.mean( np.subtract(model, pythia) ) )
        print( model_name, "- RMS of pT difference in sample: ", np.std( np.subtract(model, pythia) ) )
        print( model_name, "- mean relative error in sample: ", mean_absolute_error( model, pythia ) )

''' makes a binning out of real data with arbitrary bin width, useful
    for me to bin by jet pt in my analysis. Throws away events that are not in
    the range [min, max] '''
def split_by_bin( df, column, n_bins, min, max ):
  
    width = float(max-min) / n_bins
    low = [ min+x*width for x in range(n_bins) ]
    high = [ min+(x+1.0)*width for x in range(n_bins) ]
    df_list = []
    for i in range(len(low)):
      tmp = df[np.multiply( df[column] >= low[i], df[column] < high[i] )]
      if tmp.size == 0:
        continue
      df_list.append( tmp )
    return df_list


''' takes a list of dataframes, and returns another pandas dataframe
    such that there are approximately equivalent numbers of events of each frame
    Obviously, it reduces the dataset size, especially since
    I'm using it to equalize what starts off as a negative exponential'''
def recombine_bins_equal_entries( df_list, random_state=None ):
    if random_state == None:
        random_state = 420
    # select the number of events to use by
    # taking the minimum
    n_events = min( [ x.shape[0] for x in df_list  ] )
    df = pandas.DataFrame()
    rmdr = pandas.DataFrame()
    for x in df_list:
      # first shuffle the dataframe
      x = x.sample(frac=1, random_state=random_state)
      # take the first n_events and add the rest to remainder
      df = df.append( x.take( [ i for i in range(n_events) ], axis=0 ) )
      rmdr = rmdr.append( x.take( [ i for i in range( n_events, x.shape[0] ) ], axis=0 ) )
    # and shuffle the results before returning
    return df.sample(frac=1, random_state=random_state), rmdr.sample(frac=1, random_state=random_state)



