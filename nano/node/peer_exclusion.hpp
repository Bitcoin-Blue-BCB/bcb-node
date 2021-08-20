#pragma once

#include <nano/lib/locks.hpp>
#include <nano/node/common.hpp>

#include <boost/asio/ip/basic_endpoint.hpp>
#include <boost/asio/ip/impl/address.ipp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/identity_fwd.hpp>
#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/tag.hpp>
#include <boost/multi_index_container.hpp>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>

namespace mi = boost::multi_index;

namespace nano
{
class container_info_component;
class peer_exclusion final
{
	class item final
	{
	public:
		item () = delete;
		std::chrono::steady_clock::time_point exclude_until;
		decltype (std::declval<nano::tcp_endpoint> ().address ()) address;
		uint64_t score;
	};

	// clang-format off
	class tag_endpoint {};
	class tag_exclusion {};
	// clang-format on

public:
	// clang-format off
	using ordered_endpoints = boost::multi_index_container<peer_exclusion::item,
	mi::indexed_by<
		mi::ordered_non_unique<mi::tag<tag_exclusion>,
			mi::member<peer_exclusion::item, std::chrono::steady_clock::time_point, &peer_exclusion::item::exclude_until>>,
		mi::hashed_unique<mi::tag<tag_endpoint>,
			mi::member<peer_exclusion::item, decltype(peer_exclusion::item::address), &peer_exclusion::item::address>>>>;
	// clang-format on

private:
	ordered_endpoints peers;
	mutable nano::mutex mutex;

public:
	constexpr static size_t size_max = 5000;
	constexpr static double peers_percentage_limit = 0.5;
	constexpr static uint64_t score_limit = 2;
	constexpr static std::chrono::hours exclude_time_hours = std::chrono::hours (1);
	constexpr static std::chrono::hours exclude_remove_hours = std::chrono::hours (24);

	uint64_t add (nano::tcp_endpoint const &, size_t const);
	bool check (nano::tcp_endpoint const &);
	void remove (nano::tcp_endpoint const &);
	size_t limited_size (size_t const) const;
	size_t size () const;

	friend class telemetry_remove_peer_different_genesis_Test;
	friend class telemetry_remove_peer_different_genesis_udp_Test;
	friend class telemetry_remove_peer_invalid_signature_Test;
	friend class peer_exclusion_validate_Test;
};
std::unique_ptr<container_info_component> collect_container_info (peer_exclusion const & excluded_peers, std::string const & name);
}
