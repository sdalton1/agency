#pragma once

#include <agency/detail/config.hpp>
#include <agency/detail/requires.hpp>
#include <agency/detail/is_call_possible.hpp>
#include <agency/cuda/detail/feature_test.hpp>
#include <agency/cuda/detail/terminate.hpp>
#include <agency/cuda/device.hpp>
#include <agency/cuda/detail/future/event.hpp>
#include <agency/cuda/detail/future/stream.hpp>
#include <type_traits>


namespace agency
{
namespace cuda
{
namespace detail
{


template<class T>
inline __host__ __device__
size_t align_up(size_t offset)
{
  constexpr size_t alignment = std::alignment_of<T>::value;
  return alignment * ((offset + (alignment - 1)) / alignment);
}


inline void setup_kernel_arguments(size_t){}

template<class Arg1, class... Args>
void setup_kernel_arguments(size_t offset, const Arg1& arg1, const Args&... args)
{
  offset = align_up<Arg1>(offset);

  cudaSetupArgument(arg1, offset);
  setup_kernel_arguments(offset + sizeof(Arg1), args...);
}


template<class T1, class... Ts>
struct first_type
{
  using type = T1;
};


template<class Arg1, class... Args>
__host__ __device__
auto first_parameter(Arg1&& arg1, Args&&...)
  -> decltype(std::forward<Arg1>(arg1))
{
  return std::forward<Arg1>(arg1);
}

template<class GlobalFunctionPointer, class... Args,
         __AGENCY_REQUIRES(
           std::is_pointer<GlobalFunctionPointer>::value
         ),
         __AGENCY_REQUIRES(
           agency::detail::is_call_possible<GlobalFunctionPointer, Args...>::value
         )>
__host__ __device__
cudaError_t launch_kernel_impl(GlobalFunctionPointer kernel, ::dim3 grid_dim, ::dim3 block_dim, size_t shared_memory_size, cudaStream_t stream, const Args&... args)
{
  // reference the kernel to encourage the compiler not to optimize it away
  workaround_unused_variable_warning(kernel);

#if __cuda_lib_has_cudart
  // gracefully ignore empty launches
  if(grid_dim.x * grid_dim.y * grid_dim.z * block_dim.x * block_dim.y * block_dim.z == 0)
  {
    return cudaSuccess;
  }

#  ifndef __CUDA_ARCH__
  cudaConfigureCall(grid_dim, block_dim, shared_memory_size, stream);
  detail::setup_kernel_arguments(0, args...);

  // XXX we should use cudaLaunchKernel
  return cudaLaunch(kernel);
#  else
  // XXX generalize to multiple arguments
  if(sizeof...(Args) != 1)
  {
    return cudaErrorNotSupported;
  }

  using Arg = typename first_type<Args...>::type;

  void *param_buffer = cudaGetParameterBuffer(std::alignment_of<Arg>::value, sizeof(Arg));
  std::memcpy(param_buffer, &detail::first_parameter(args...), sizeof(Arg));

  void* void_kernel_pointer = reinterpret_cast<void*>(kernel);

  return cudaLaunchDevice(void_kernel_pointer, param_buffer, grid_dim, block_dim, shared_memory_size, stream);
#  endif // __CUDA_ARCH__
#else // __cuda_lib_has_cudart
  return cudaErrorNotSupported;
#endif
}


template<class GlobalFunctionPointer, class... Args,
         __AGENCY_REQUIRES(
           std::is_pointer<GlobalFunctionPointer>::value
         ),
         __AGENCY_REQUIRES(
           agency::detail::is_call_possible<GlobalFunctionPointer, Args...>::value
         )>
__host__ __device__
cudaError_t launch_kernel(GlobalFunctionPointer kernel, ::dim3 grid_dim, ::dim3 block_dim, size_t shared_memory_size, cudaStream_t stream, const Args&... args)
{
  struct workaround
  {
    __host__ __device__
    static cudaError_t supported_path(GlobalFunctionPointer kernel, ::dim3 grid_dim, ::dim3 block_dim, size_t shared_memory_size, cudaStream_t stream, const Args&... args)
    {
      // reference the kernel to encourage the compiler not to optimize it away
      detail::workaround_unused_variable_warning(kernel);

      return launch_kernel_impl(kernel, grid_dim, block_dim, shared_memory_size, stream, args...);
    }

    __host__ __device__
    static cudaError_t unsupported_path(GlobalFunctionPointer kernel, ::dim3, ::dim3, size_t, cudaStream_t, const Args&...)
    {
      // reference the kernel to encourage the compiler not to optimize it away
      detail::workaround_unused_variable_warning(kernel);

      return cudaErrorNotSupported;
    }
  };

#if __cuda_lib_has_cudart
  cudaError_t result = workaround::supported_path(kernel, grid_dim, block_dim, shared_memory_size, stream, args...);
#else
  cudaError_t result = workaround::unsupported_path(kernel, grid_dim, block_dim, shared_memory_size, stream, args...);
#endif

  return result;
}


template<class GlobalFunctionPointer, class... Args,
         __AGENCY_REQUIRES(
           std::is_pointer<GlobalFunctionPointer>::value
         ),
         __AGENCY_REQUIRES(
           agency::detail::is_call_possible<GlobalFunctionPointer, Args...>::value
         )>
__host__ __device__
void try_launch_kernel(GlobalFunctionPointer kernel, ::dim3 grid_dim, ::dim3 block_dim, size_t shared_memory_size, cudaStream_t stream, const Args&... args)
{
  // the error message we return depends on how the program was compiled
  const char* error_message = 
#if __cuda_lib_has_cudart
   // we have access to CUDART, so something went wrong during the kernel
#  ifndef __CUDA_ARCH__
   "cuda::detail::checked_launch_kernel(): CUDA error after cudaLaunch()"
#  else
   "cuda::detail::checked_launch_kernel(): CUDA error after cudaLaunchDevice()"
#  endif // __CUDA_ARCH__
#else // __cuda_lib_has_cudart
   // we don't have access to CUDART, so output a useful error message explaining why it's unsupported
#  ifndef __CUDA_ARCH__
   "cuda::detail::checked_launch_kernel(): CUDA kernel launch from host requires nvcc"
#  else
   "cuda::detail::checked_launch_kernel(): CUDA kernel launch from device requires arch=sm_35 or better and rdc=true"
#  endif // __CUDA_ARCH__
#endif
  ;

  detail::throw_on_error(launch_kernel(kernel, grid_dim, block_dim, shared_memory_size, stream, args...), error_message);
}


template<class GlobalFunctionPointer, class... Args,
         __AGENCY_REQUIRES(
           std::is_pointer<GlobalFunctionPointer>::value
         ),
         __AGENCY_REQUIRES(
           agency::detail::is_call_possible<GlobalFunctionPointer, Args...>::value
         )>
__host__ __device__
void try_launch_kernel_on_device(GlobalFunctionPointer kernel, ::dim3 grid_dim, ::dim3 block_dim, size_t shared_memory_size, cudaStream_t stream, int device, const Args&... args)
{
  detail::scoped_current_device scope(device);

  try_launch_kernel(kernel, grid_dim, block_dim, shared_memory_size, stream, args...);
}


template<class GlobalFunctionPointer, class... Args,
         __AGENCY_REQUIRES(
           std::is_pointer<GlobalFunctionPointer>::value
         ),
         __AGENCY_REQUIRES(
           agency::detail::is_call_possible<GlobalFunctionPointer, Args...>::value
         )>
__host__ __device__
detail::event
  then_launch_kernel(GlobalFunctionPointer kernel, ::dim3 grid_dim, ::dim3 block_dim, size_t shared_memory_size, detail::event& predecessor, const device_id& device, const Args&... args)
{
  // make a stream for the continuation and invalidate the predecessor
  detail::stream new_stream = predecessor.make_dependent_stream_and_invalidate(device);

  // launch the kernel on the new stream
  try_launch_kernel_on_device(kernel, grid_dim, block_dim, shared_memory_size, new_stream.native_handle(), new_stream.device().native_handle(), args...);

  // return a new event
  return detail::event(std::move(new_stream));
}


} // end detail
} // end cuda
} // end agency

