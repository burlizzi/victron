    // Existing powerOut_ publishing block

    if (acPower_ != nullptr) {
        acPower_->publish_state(float(ACPower));
    } // New lines added
