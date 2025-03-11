// SPDX-License-Identifier: GPL-2.0-or-later

#[cfg(not(target_os = "linux"))]
compile_error!("This script must target Linux systems.");

use byte_unit::{Byte, UnitType};
use once_cell::sync::Lazy;
use proc_getter::buddyinfo::*;

#[derive(tabled::Tabled)]
struct ZoneEntry {
    size: String,
    fragments: usize,
    available: String,
}

impl ZoneEntry {
    fn new(i: usize, e: usize) -> Result<(Self, usize), Box<dyn std::error::Error>> {
        let size = *PAGE_SIZE << i;
        let avail = size * e;
        Ok((
            Self {
                size: Byte::from_u128(size as u128)
                    .ok_or("Could not parse iteration")?
                    .get_appropriate_unit(UnitType::Binary)
                    .to_string(),
                fragments: e,
                available: format!(
                    "{:.1}",
                    Byte::from_u128((size * e) as u128)
                        .ok_or("Could not parse total availablity")?
                        .get_appropriate_unit(UnitType::Binary)
                ),
            },
            avail,
        ))
    }
}

static PAGE_SIZE: Lazy<usize> = Lazy::new(|| page_size::get());

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let zones = buddyinfo().or(Err("Could not read buddyinfo"))?;
    println!(
        "Page size = {:.1}",
        Byte::from_u128(*PAGE_SIZE as u128)
            .ok_or("Could not parse page size")?
            .get_appropriate_unit(UnitType::Binary)
    );
    let mut cx: usize = 0;
    for zone in zones {
        println!("Zone {} - {}", zone.node(), zone.zone());
        let mut output: Vec<ZoneEntry> = vec![];
        let mut ax: usize = 0;
        for (i, e) in zone.page_nums().iter().enumerate() {
            let res = ZoneEntry::new(i, *e)?;
            output.push(res.0);
            ax += res.1;
        }
        println!(
            "{}\nTotal available in zone: {:.1}\n",
            tabled::Table::new(output),
            Byte::from_u128(ax as u128)
                .ok_or("Could not parse zone-available")?
                .get_appropriate_unit(UnitType::Binary)
        );
        cx += ax;
    }
    println!(
        "Total available: {:.1}",
        Byte::from_u128(cx as u128)
            .ok_or("Could not parse total-available")?
            .get_appropriate_unit(UnitType::Binary)
    );
    Ok(())
}
