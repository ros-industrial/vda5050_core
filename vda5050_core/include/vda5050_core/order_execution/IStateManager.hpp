#ifndef VDA5050_CORE__ORDER_EXECUTION__I_STATE_MANAGER_HPP_
#define VDA5050_CORE__ORDER_EXECUTION__I_STATE_MANAGER_HPP_

#include <cstdint>
#include <string>

#include "vda5050_core/order_execution/Order.hpp"

class IStateManager
{
public:
  virtual ~IStateManager() = default;

  virtual std::string last_node_id() const = 0;
  virtual uint32_t last_node_sequence_id() const = 0;
  virtual bool node_states_empty() const = 0;
  virtual bool action_states_still_executing() const = 0;

  virtual void cleanup_previous_order() = 0;
  virtual void set_new_order(const vda5050_core::order::Order& order) = 0;
  virtual void clear_horizon() = 0;
  virtual void append_states_for_update(
    const vda5050_core::order::Order& order_update) = 0;
  virtual void update_current_order(
    const vda5050_core::order::Order& order_update) = 0;
};

#endif  // VDA5050_CORE__ORDER_EXECUTION__I_STATE_MANAGER_HPP_
