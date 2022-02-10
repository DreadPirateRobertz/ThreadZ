make
if [ $? == 0 ]
then 

touch test/output_tester # make a file for us to put stuff into it

echo | > test/output_tester
echo
echo Running tiny input in debug with 2 thread 
./pzip test/input_tiny test/output_tester 2 --debug
# aaeeoooooeee a 2 e 5 o 5
cat test/output_tester
echo The above is the output of the test, is it the same as the test case below?
cat test/output_tiny_2t_debug

echo
echo Running tiny input in zipped with 2 thread 
./pzip test/input_tiny test/output_tester 2 
# aaeeoooooeee a 2 e 5 o 5
cat test/output_tester
echo
echo The above is the output of the test, is it the same as the test case below?
cat test/output_tiny_2t

echo
echo Running small input in debug with 4 thread
./pzip test/input_small test/output_tester 4 --debug
cat test/output_tester
echo The above is the output of the test, is it the same as the test case below?
cat test/output_small_4t_debug

cat test/output_small_4t > test/output_tester
echo
echo Running small input in zipped with 4 thread
./pzip test/input_small test/output_tester 4 
cat test/output_tester
echo
echo The above is the output of the test, is it the same as the test case below?
cat test/output_small_4t

fi 

