cat parts/*.txt > parts/merged.txt
diff input1.txt parts/merged.txt
rm parts/*
