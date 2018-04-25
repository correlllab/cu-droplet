from __future__ import print_function
import SocketConnection as sc
import time
import atexit
import random

rr = sc.RR_API()
rr.Connect("localhost")

def get_rr_blob_count():
    blobCountStr = rr.GetVariable("BLOB_COUNT")
    if blobCountStr is "":
        return 0
    else:
        return int(blobCountStr)

@atexit.register
def goodbye():
    rr.close()

tau=5
theta=10.0
total_run_time = 60*30

with open("C:\\Users\\Colab\\Desktop\\dat_tau_%d_theta_%.1f.csv"%(tau, theta),'w') as f:
    try:
        last_num_blobs=0
        f.write("num_blobs, elapsed_time\n")
        start_time = time.time()
        stop_time = start_time+total_run_time
        while(time.time()<stop_time):
            num_blobs = get_rr_blob_count()
            if num_blobs is not last_num_blobs:
                print("%d, %.4f"%(num_blobs, time.time()-start_time))
                f.write("%d, %.4f\n"%(num_blobs, time.time()-start_time))
                f.flush()
                last_num_blobs=num_blobs 
            rr.WaitImage(10)
    except KeyboardInterrupt:
        f.flush()
        goodbye()

