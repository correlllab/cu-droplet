import numpy as np

# =======================================================
# Pick the entering index using this variety of functions
# =======================================================
def largest_coeff(obj_coeffs, nonbas):
    return np.argmax(obj_coeffs)

def smallest_pos_coeff(obj_coeffs, nonbas):
    curr_min = np.inf
    curr_pos = 0
    for x in enumerate(obj_coeffs):
        if(x[1] > 1e-10 and x[1] < curr_min):
            (curr_pos, curr_min) = x
    
    return curr_pos

def bland_rule(obj_coeffs, nonbas):
    pos_id_list = []
    for x in enumerate(obj_coeffs):
        if(x[1] > 1e-10):
            pos_id_list.append(x[0])

    sub_nonbas = np.take(nonbas, pos_id_list)
    min_id = np.amin(sub_nonbas)
    nonbas_min_id = np.where(nonbas == min_id)[0][0]
    
    return nonbas_min_id

def random(obj_coeffs, nonbas):
    
    while(True):
        enter_id = np.random.random_integers(0, obj_coeffs.size - 1)
        if(obj_coeffs[enter_id] > 1e-10):
            break

    return enter_id


# =========================================
# Standard pivot without using eta factors
# =========================================
def pivot(A, b, c, bas, nonbas, verbose = False):
    """ Function: pivot (A,b,c,bas,nonbas)
    Inputs: 
    (A,b,c) = the original problem is assumed to be of the form max c' x s.t. Ax = b, x >= 0
    bas, nonbas = current list of basic and non-basic indices. 
    Returns:
    out_bas, out_nonbas = new lists of basic and non-basic indices
    out_z = new objective value
    is_final = the given dictionary is actually final (termination condition)
    is_unbounded = the given dictionary is actually unbounded (termination condition) 
    """

    (m, n) = A.shape

    is_final = 0
    is_unbounded = 0

    # Select the sub-matrices (columns) for Ab, Ai
    B = A[:,bas]
    N = A[:,nonbas]

    # Split the objective row coefficients
    c_b = np.take(c, bas)
    c_n = np.take(c, nonbas)

    # Compute p = c_b . Inv(B)
    p = np.linalg.solve(B.transpose(), c_b)

    # Initialize output variables for next dictionary
    out_bas = np.array(bas)
    out_nonbas = np.array(nonbas)
    out_z = np.dot(p, b) # the current objective value

    # Compute objective row coefficients
    obj_coeffs = c_n - np.dot(p, N)

    # Choose largest ceoff and its index as entering variable 
    c_max = np.amax(obj_coeffs)

    # If highest coeff is 0 then we are done
    if(c_max <= 1e-10):
        print('Simple Pivot: Final dictionary found. Objective value is %f'%out_z)
        return (out_bas, out_nonbas, out_z, True, False)
    
    enter_id = np.argmax(obj_coeffs)
    if(verbose):
        print('Simple Pivot: Entering Index = %d'%nonbas[enter_id])

    # --- Leaving Variable Analysis --- 
    # Compute dict column corresponding to entering variable analysis
    # Compute A_j and b_hat columns
    A_j = np.linalg.solve(B, -A[:,nonbas[enter_id]])
    b_hat = np.linalg.solve(B, b)

    # Init leaving variable index
    leave_id = -1
    leave_lim = np.inf

    # Search for leaving variable
    for i in range(m):
        if(A_j[i] < 0):
            l_coeff = -b_hat[i] / A_j[i]
            if(l_coeff < leave_lim or (l_coeff >= (leave_lim - 1e-8) and l_coeff <= (leave_lim + 1e-8) and bas[i] <= bas[leave_id])): 
                leave_lim = l_coeff
                leave_id = i

    if(leave_id == -1):
        print('Simple Pivot: Dictionary Unbounded\n')
        return (out_bas, out_nonbas, out_z, False, True)

    # --- End Leaving Variable Analysis ---
    if(verbose):
        print('Simple Pivot: Leaving index = %d'%bas[leave_id])
    
    # Swap out the entering and leaving variable indices
    out_nonbas[enter_id] = bas[leave_id]
    out_bas[leave_id] = nonbas[enter_id]

    # Update the objective value
    out_z += obj_coeffs[enter_id] * leave_lim

    # Thats all folks!
    return (out_bas, out_nonbas, out_z, False, False)



# ===========================
# Pivoting using eta matrices
# ===========================
def pivot_with_eta_factors(A, b, c, bas, nonbas, eta_file, _enter_id_picker = largest_coeff, verbose = False):
    """ Function: pivot_with_eta_factors (A,b,c,bas,nonbas)
    Inputs: 
    (A,b,c) = the original problem is assumed to be of the form max c' x s.t. Ax = b, x >= 0
    bas, nonbas = current set of basic and non-basic indices.
    etafile = current set of eta matrices. Stored as a numpy object list
    _enter_id_picker = the function to use to pick entering index. Defaults to largest coefficient (largest_coeff)
    _leaving_id_picker = the function to use to pick leaving index. Defaults to largest objective value increase (largest_inc)

    Returns:
    out_bas, out_nonbas = new set of basic and non-basic indices
    out_z = new objective value
    out_etafile = updated eta file with new E matrix added for this step
    is_final = the given dictionary is actually final (termination condition)
    is_unbounded = the given dictionary is actually unbounded (termination condition)

    """
    if(verbose):
        print '\n-------------------------'

    (m, n) = A.shape

    is_final = 0
    is_unbounded = 0

    # Select the sub-matrices (columns) for Ab, Ai
    B_inv = eta_file
    N = A[:,nonbas]

    # Split the objective row coefficients
    c_b = np.take(c, bas)
    c_n = np.take(c, nonbas)

    # Compute p = c_b . Inv(B)
    #p = np.linalg.solve(B.transpose(), c_b)
    p = np.dot(B_inv.transpose(), c_b)

    # Initialize output variables for next dictionary
    out_bas = np.array(bas)
    out_nonbas = np.array(nonbas)
    out_z = np.dot(p, b) # the current objective value

    # Compute objective row coefficients
    obj_coeffs = c_n - np.dot(p, N)
    if(verbose):
        print 'Basic variable indices - '
        print bas
        print 'Non-basic variable indices - '
        print nonbas
        print 'Objective row coefficients - '
        print obj_coeffs

    # Choose largest ceoff and its index as entering variable 
    c_max = np.amax(obj_coeffs)

    # If highest coeff is 0 then we are done
    if(c_max <= 1e-10):
        if(verbose):
            print('Eta Pivot: Final dictionary found. Objective value is %f'%out_z)
        return (out_bas, out_nonbas, out_z, True, False, eta_file)
    
    # Select the entering index
    enter_id = _enter_id_picker(obj_coeffs, nonbas)
    if(verbose):
        print('Eta Pivot: Entering Index = %d'%nonbas[enter_id])

    # --- Leaving Variable Analysis --- 
    # Compute dict column corresponding to entering variable analysis
    # Compute A_j and b_hat columns
    delta_x_b = np.dot(B_inv, A[:,nonbas[enter_id]])
    A_j = -delta_x_b
    b_hat = np.dot(B_inv, b)

    # Init leaving variable index
    leave_id = -1
    leave_lim = np.inf

    # Search for leaving variable
    for i in range(m):
        if(A_j[i] < 0):
            l_coeff = -b_hat[i] / A_j[i]
            if(l_coeff < leave_lim or (l_coeff >= (leave_lim - 1e-8) and l_coeff <= (leave_lim + 1e-8) and bas[i] <= bas[leave_id])): 
                leave_lim = l_coeff
                leave_id = i

    if(leave_id == -1):
        print('Eta Pivot: Dictionary Unbounded\n')
        return (out_bas, out_nonbas, out_z, False, True, eta_file)

    # --- End Leaving Variable Analysis ---
    if(verbose):
        print('Eta Pivot: Leaving index = %d'%bas[leave_id])
    
    # Swap out the entering and leaving variable indices
    out_nonbas[enter_id] = bas[leave_id]
    out_bas[leave_id] = nonbas[enter_id]

    # Update the objective value
    out_z += obj_coeffs[enter_id] * leave_lim

    # Update the eta file
    (bm, bn) = B_inv.shape
    E_new_inv = np.eye(bn, bm)
    delta_x_b[leave_id] = -1 # This line is found by simplifying 1 - (delta_x_b[leave_id] - 1) / delta_x_b[leave_id]
    E_new_inv[:,leave_id] = delta_x_b / A_j[leave_id]
    eta_file = np.dot(E_new_inv, B_inv)

    # Thats all folks!
    if(verbose):
        print '-------------------------\n'
    return (out_bas, out_nonbas, out_z, False, False, eta_file)