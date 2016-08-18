/*
	MATHS.H
	-------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)

	Originally from the ATIRE codebase.
*/

namespace JASS
	{
	/*
		CLASS MATHS
		-----------
	*/
	class maths
		{
		private:
			maths() {}
			
		public:
			/*
				MAX()
				-----
				Return the maximum of two integers (order preserving)
			*/
			template <typename TYPE>
			static const TYPE &						// [out] The maximum of the parameters
			max
				(
				const TYPE &first,			// [in] first of the two
				const TYPE &second			// [in] second of the two
				)
				{
				return first >= second ? first : second;
				}
			
			/*
				MAX()
				-----
				Return the maximum of three integers (order preserving)
			*/
			template <typename TYPE>
			static const TYPE &						// [out] The maximum of the parameters
			max
				(
				const TYPE &first,			// [in] first of the three
				const TYPE &second,			// [in] second of the three
				const TYPE &third				// [in] third of the three
				)
				{
				return max(max(first, second), third);
				}

			/*
				MAX()
				-----
				Return the minimum of two integers (order preserving)
			*/
			template <typename TYPE>
			static const TYPE &						// [out] The minimum of the parameters
			min
				(
				const TYPE &first,			// [in] first of the two
				const TYPE &second			// [in] second of the two
				)
				{
				return first <= second ? first : second;
				}
				
			/*
				MAX()
				-----
				Return the minimum of three integers (order preserving)
			*/
			template <typename TYPE>
			static const TYPE &						// [out] The minimum of the parameters
			min
				(
				const TYPE &first,			// first of the three
				const TYPE &second,			// second of the three
				const TYPE &third				// third of the three
				)
				{
				return min(min(first, second), third);
				}
		};
	}


