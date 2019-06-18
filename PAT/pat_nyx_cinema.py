#! /usr/bin/env python

import argparse, os, csv

from pat import file_utilities as futils
from pat import plot_utilities as putils
from pat import cinema
from pat import Job as j



class pat_nyx_cinema(cinema.CinemaWorkflow):

	def prepare_cinema(self):
		# Open CSV file
		metrics_csv 	 = self.json_data["project-home"] + "/cbench/" + self.json_data['output']['metricsfname'] + ".csv"
		output_file_name = self.json_data["project-home"] + "/cbench/" + "data.csv"
		#reader = futils.open_csv_file(metrics_csv)
		print metrics_csv
		print output_file_name

		all = []
		with open(metrics_csv,'r') as csvinput:
			reader = csv.reader(csvinput)

			# Modify Cinema files
		
			row = next(reader)
			row.append('FILE_SimStats_Pk')
			#row.append('FILE_lya_all_axes_x_Pk')
			#row.append('FILE_lya_all_axes_y_Pk')
			#row.append('FILE_lya_all_axes_z_Pk')
			all.append(row)

			values = ["sim_stats_rhob.png", "sim_stats_rhodm.png", "sim_stats_temp.png", "sim_stats_velmag.png", "sim_stats_velmag.png", "sim_stats_vz.png"]
			count = 0
			for row in reader:
				row.append(values[count])
				#row.append("lya_all_axes_x.png")
				#row.append("lya_all_axes_y.png")
				#row.append("lya_all_axes_z.png")
				all.append(row)

				count = count + 1
				if (count == 6):
					count = 0

			
			futils.write_csv(output_file_name, all)


	def create_plots(self):
		output_path = self.json_data['project-home'] + "/cinema"
		csv_file_path = self.json_data['output']['run-path'] + self.json_data['output']['metricsfname'] + ".csv"
		x_range = self.json_data['simulation-analysis']['plotting']['x-range']

		for ana in self.json_data['simulation-analysis']['analysis']:
			plot_title = ana['title']
			to_plot = []  # all the items to plot

			k_list = []
			orig_pk = []

			# Find the original file
			for file in ana['files']:
				if (file['name']=="orig"):
					k_list  = futils.extract_csv_col(file['path'], ' ', 2)
					orig_pk = futils.extract_csv_col(file['path'], ' ', 3)

			for file in ana['files']:
				if (file['name']!="orig"):
					print (file['path'])

					temp_pk = futils.extract_csv_col(file['path'], ' ', 3)
					if (temp_pk is not None):
						pk_ratio = [i / j for i, j in zip(temp_pk, orig_pk)]
						this_tuple = (pk_ratio, file['name']) #array, name
						to_plot.append(this_tuple)

			putils.plotScatterGraph(k_list, 'k', 'pk', plot_title, output_path, x_range, to_plot)



# Parse Input
parser = argparse.ArgumentParser()
parser.add_argument("--input-file")
opts = parser.parse_args()


# Creat Ciname DB
cinema = pat_nyx_cinema( opts.input_file )
cinema.create_plots()
cinema.create_cinema()