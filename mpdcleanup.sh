echo "glx1 cleanup"
ssh rmarks@glx1 mpdcleanup
echo "glx2 cleanup"
ssh rmarks@glx2 mpdcleanup
echo "glx3 cleanup"
ssh rmarks@glx3 mpdcleanup
echo "glx4 cleanup"
ssh rmarks@glx4 mpdcleanup
echo "glx5 cleanup"
ssh rmarks@glx5 mpdcleanup
echo "glx4 boot"
ssh rmarks@glx4 mpdboot ‐n 5 ‐f ./machinefile.txt
