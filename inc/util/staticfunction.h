/*
 * staticfunction.h
 *
 *  Created on: 08.01.2018
 *      Author: Hannes
 */

#ifndef STATICFUNCTION_H_
#define STATICFUNCTION_H_

#include <type_traits>
#include <utility>
#include <cstddef>
#include <new>


template<class TSignature, std::size_t TSize = sizeof(void*) * 3>
class StaticF;

template<class TRet, class... TArgs, std::size_t TSize>
class StaticF<TRet (TArgs...), TSize>
{
private:
	class Invoker {
	public:
		virtual ~Invoker(){}
		virtual TRet exec(TArgs... args) const = 0;
		virtual TRet exec(TArgs... args) = 0;
	};

	template <class TBound>
	class InvokerBound : public Invoker {
	public:
		template <class TFunc>
		InvokerBound(TFunc&& func) : _func(func) {}
		virtual void exec(TArgs... args) {
			//return
					_func(std::forward<TArgs>(args)...);
		}
		virtual void exec(TArgs... args) const {
			//return
					_func(std::forward<TArgs>(args)...);
		}
		virtual ~InvokerBound(){}
		TBound _func;
	};

	static const std::size_t size = TSize + sizeof(Invoker);
	typedef typename std::aligned_storage<size, std::alignment_of<Invoker>::value>::type StorageType;
	StorageType handler_;
	bool valid;
public:
	//
	// Konstruktors
	//
	StaticF(): valid(false){}

	template<class TFunc>
	StaticF(TFunc&& func): valid(true){
		assignHandler(std::forward<TFunc>(func));
	}
	//
	//Public Interface
	//
	template<class TFunc>
	void assignHandler(TFunc&& func) {
		typedef typename std::decay<TFunc>::type DecayedFuncType;
		typedef InvokerBound<DecayedFuncType> InvokerBoundType;

		new (&handler_) InvokerBoundType(std::forward<TFunc>(func));
		valid = true;

		 static_assert(sizeof(InvokerBoundType) <= size,
		"Increase the TSize template argument of the StaticFucntion");
	}
	//Call to function
	TRet operator() (TArgs... args) {
		if(valid){
		auto invoker = reinterpret_cast<Invoker*>(&handler_);
		return invoker->exec(std::forward<TArgs>(args)...);
		}
	}
	TRet operator() (TArgs... args) const{
		if(valid){
		auto invoker = reinterpret_cast<Invoker*>(&handler_);
		return invoker->exec(std::forward<TArgs>(args)...);
		}
	}
};



#endif /* STATICFUNCTION_H_ */
