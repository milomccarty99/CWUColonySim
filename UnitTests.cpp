#include "CWUColonySim.cpp"

bool test_reset_field_map()
{
	cout << "these should all be u" << endl;
	reset_field_map();
	display_field_map();
	//visual check
	return true;
}
int main()
{
	test_reset_field_map();
}