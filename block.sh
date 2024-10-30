#!/bin/bash
#
#SBATCH --cpus-per-task=4
#SBATCH --time=02:00
#SBATCH --mem=2G
#SBATCH --partition=slow

srun ./one_lock_blocking_queue_throughput --n_producers 2 --n_consumers 2 --seconds 5 --init_allocator 100000000