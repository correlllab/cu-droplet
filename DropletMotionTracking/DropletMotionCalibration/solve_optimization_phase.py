import numpy as np
import Simplex as sp
import time

def solve_optimization_phase(P, q, r, enter_id_picker = sp.largest_coeff, vb = True, pivot_vb = False):
    """ Function: solve_optimization_phase(P, q, r, enter_id_picker, vb, pivot_vb)
    Solves the optimization problem: (max) r.x subject to P.x <= q, x >=0 using Simplex.
    enter_id_picker = The function used to select the entering variable for each step of Simplex. Refer to Simplex.py for a list of available functions.
    Returns: (out_x, z, count)
    out_x - The optimized x vector
    z - The objective value
    count - The number of pivots needed by the solver to arrive at the solution
    """
    (m, n) = P.shape
    A = np.hstack((P, np.eye(m)))
    c = np.hstack((r, np.zeros(m)))
    eta_file = np.eye(m)
    bas = np.arange(n, n+m)
    nonbas = np.arange(0, n)
    final = False
    unbd = False

    # TODO : Test for infeasible starting dictionary and do initialization phase
    if(np.max(c) <= 1e-10):
        print('The Starting Dictionary is infeasible.')
        assert True
            
    count = 0
    if(vb):
        timing = 0.0

    while(not final and not unbd):
        if(vb):
            time_start = time.time()
        (bas, nonbas, z, final, unbd, eta_file) = sp.pivot_with_eta_factors(A, q, c, bas, nonbas, eta_file, _enter_id_picker = enter_id_picker, verbose = pivot_vb)
        if(vb):
            timing += time.time() - time_start
        count += 1

    # Find the values for the decision variables
    if(not unbd):
        b_hat = np.dot(eta_file, q)
        out_x = np.zeros(m + n)
        np.put(out_x, bas, b_hat)
    else:
        return (np.array([]), np.inf, count) # Problem is unbounded

    if(vb):
        print('\n===== TIMING INFORMATION =====\n')
        print('Number of pivots = %d'%count)
        print('Time = %fs'%timing)
        print('\n==============================\n')

    return (out_x[:n], z, count)